#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Shared DisCostiC simulation job workflow: modules, lock, ECM scratch, build, Kerncraft, and run.

set -euo pipefail
: "${TC_STAGE_DIR:?TC_STAGE_DIR must be set}"
: "${TC_PROCNUMBERS:?TC_PROCNUMBERS must be set}"
: "${TC_CONFIG_FILE:?TC_CONFIG_FILE must be set}"
: "${TC_SIM_OUT:?TC_SIM_OUT must be set}"

_COMMON_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TC_ROOT="$(cd "${_COMMON_SCRIPT_DIR}/../.." && pwd)"
ROOT="${DISCOSTIC_ROOT:-${DISCOSTIC_EXTENSION_ROOT:-$(cd "${TC_ROOT}/../.." && pwd)}}"
LOCKFILE="${TC_ROOT}/.discostic_workspace.lock"
JOB_ID="${SLURM_JOB_ID:-local}"
JOB_CFG="${TC_ROOT}/job_configs/job_${JOB_ID}.cfg"

module load python git cmake likwid intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null \
    || module load python git cmake likwid intel/2025.2.0 intelmpi/2021.16.0
VENV="${ROOT}/.venv-discostic"
[ ! -d "$VENV" ] && /usr/bin/python3 -m venv "$VENV"
source "$VENV/bin/activate"
python -c "import kerncraft" 2>/dev/null || python -m pip install --quiet kerncraft numpy osaca

cd "${ROOT}"
mkdir -p "${TC_ROOT}/job_configs" "${TC_SIM_OUT}"
exec 200>"${LOCKFILE}"
flock -x 200
cp "${TC_CONFIG_FILE}" "${JOB_CFG}"
cp "${JOB_CFG}" config.cfg

FILENAME=$(awk -F= '/^benchmark_kernel/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
PART2=$(awk -F= '/^part2/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)

if [ -n "${DISCOSTIC_ECM_SUBDIR:-}" ]; then
    mkdir -p "${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR}"
    sed -i "s/^benchmark_kernel = .*/benchmark_kernel = ${DISCOSTIC_ECM_SUBDIR}\/${FILENAME}/" config.cfg
    export DISCOSTIC_KERNCRAFT_SCRATCH="${ROOT}/nodelevel/kernels/kerncraft-scratch/${DISCOSTIC_ECM_SUBDIR}"
    mkdir -p "${DISCOSTIC_KERNCRAFT_SCRATCH}"
fi

KERN_DEFAULT="${ROOT}/nodelevel/kernels/.stream-triad.c_kerncraft"
_restore_kerncraft_scratch() {
    if [ -n "${DISCOSTIC_KERNCRAFT_SCRATCH:-}" ] && [ -L "${KERN_DEFAULT}" ]; then
        rm -f "${KERN_DEFAULT}"
        if [ -d "${KERN_DEFAULT}.preserved-shared" ]; then
            mv "${KERN_DEFAULT}.preserved-shared" "${KERN_DEFAULT}"
        fi
    fi
}
_restore_discostic_repo() {
    # Return all tracked files the sim job touched back to git HEAD so the
    # DisCostiC-Sim repo is clean after every run.  Output files are
    # already moved to TC_SIM_OUT before this runs.
    git -C "${ROOT}" checkout -- config.cfg test/P2P.hpp 2>/dev/null || true
    git -C "${ROOT}" checkout -- nodelevel/configs/STREAM_FILE.cfg 2>/dev/null || true
    # Remove untracked per-job directories left in the main repo
    if [ -n "${DISCOSTIC_ECM_SUBDIR:-}" ]; then
        rm -rf "${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR}" 2>/dev/null || true
        rm -rf "${ROOT}/nodelevel/kernels/kerncraft-scratch/${DISCOSTIC_ECM_SUBDIR}" 2>/dev/null || true
    fi
}
_on_exit() {
    _restore_kerncraft_scratch
    _restore_discostic_repo
}
_install_kerncraft_scratch() {
    if [ -z "${DISCOSTIC_KERNCRAFT_SCRATCH:-}" ]; then
        return 0
    fi
    mkdir -p "${DISCOSTIC_KERNCRAFT_SCRATCH}"
    if [ -d "${KERN_DEFAULT}" ] && [ ! -L "${KERN_DEFAULT}" ]; then
        if [ ! -d "${KERN_DEFAULT}.preserved-shared" ]; then
            mv "${KERN_DEFAULT}" "${KERN_DEFAULT}.preserved-shared"
        fi
    fi
    rm -f "${KERN_DEFAULT}"
    ln -sfn "${DISCOSTIC_KERNCRAFT_SCRATCH}" "${KERN_DEFAULT}"
}
trap _on_exit EXIT
_install_kerncraft_scratch

rm -f "${ROOT}/nodelevel/configs/${FILENAME}"*.cfg 2>/dev/null || true
if [ -n "${DISCOSTIC_ECM_SUBDIR:-}" ]; then
    rm -f "${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR}/${FILENAME}"*.cfg 2>/dev/null || true
fi
BASE_ECM_CFG="${ROOT}/nodelevel/configs/${FILENAME}.cfg"
if [ -n "${DISCOSTIC_ECM_SUBDIR:-}" ]; then
    BASE_ECM_CFG="${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR}/${FILENAME}.cfg"
fi

ln -sfn "${FILENAME}.hpp" "${ROOT}/test/P2P.hpp"
touch "${ROOT}/include/Grid.hpp"
# Always rebuild after linking the benchmark kernel into P2P.hpp. Skipping this
# when ./discostic already exists reuses a binary compiled against a different
# kernel (e.g. HEAT) and produces zero-byte sends / zero COMP bufSize.
[ ! -d "${ROOT}/build" ] && cmake -S "${ROOT}" -B "${ROOT}/build"
make -C "${ROOT}/build" discostic
# CMake RUNTIME_OUTPUT_DIRECTORY writes to ${ROOT}/discostic; copy from build/
# only if that layout is not used.
if [ ! -x "${ROOT}/discostic" ] && [ -x "${ROOT}/build/discostic" ]; then
    cp "${ROOT}/build/discostic" "${ROOT}/discostic"
fi
if [ ! -x "${ROOT}/discostic" ]; then
    echo "ERROR: discostic binary missing after build" >&2
    exit 1
fi

# Pre-generate the ECM cfg from the shell (venv is active here) before launching MPI.
# Discostic's internal Kerncraft call via system() may fail in the MPI environment if
# the venv PATH is not propagated to child processes. By generating the cfg here and
# copying to all per-rank files, discostic can read the correct values even if its
# internal Kerncraft run fails silently.
_pre_generate_ecm_cfg() {
    local arch pmodel cpred compiler_flags_raw
    local nproc_cfg tpn N_cfg Nlocal ECM_core DEFINE_N cores_per_socket

    arch=$(awk -F= '/^micro_architecture/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    pmodel=$(awk -F= '/^pmodel/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    cpred=$(awk -F= '/^cache_predictor/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    compiler_flags_raw=$(awk '/^compiler_flags/ {sub(/^compiler_flags *=[ \t]*/,""); gsub(/#.*/,""); gsub(/[[:space:]]+$/,""); print; exit}' config.cfg)
    nproc_cfg=$(awk -F= '/^number_of_processes/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    tpn=$(awk -F= '/^task_per_node/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    N_cfg=$(awk -F= '/^N / {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)

    cores_per_socket=$(grep -m1 "^cores per socket" "${ROOT}/nodelevel/machine-files/${arch}.yml" 2>/dev/null | awk '{print $NF}')
    [ -z "${cores_per_socket}" ] && cores_per_socket="${tpn}"

    if [ "${tpn}" -lt "${cores_per_socket}" ]; then
        ECM_core="${tpn}"
    else
        ECM_core="${cores_per_socket}"
    fi
    Nlocal=$(( N_cfg / nproc_cfg ))
    DEFINE_N=$(( Nlocal * ECM_core ))

    # Pre-populate the per-job Kerncraft scratch from the shared cache so that Kerncraft
    # gets a cache hit for the compiled assembly and avoids recompiling the kernel.
    # The Intel-specific compiler flags (-fno-alias, -xCORE-AVX512, etc.) require icc;
    # seeding kernel.s lets Kerncraft skip compilation even if icc is slow to start.
    # OSACA analysis directories (auto/, OSACA/) are excluded: they may contain pickles
    # from an older osaca version, causing ModuleNotFoundError on load.  Kerncraft will
    # regenerate them fresh using the current venv's osaca.
    if [ -n "${DISCOSTIC_KERNCRAFT_SCRATCH:-}" ] && [ -d "${KERN_DEFAULT}.preserved-shared" ]; then
        cp -rp "${KERN_DEFAULT}.preserved-shared/." "${DISCOSTIC_KERNCRAFT_SCRATCH}/"
        # Strip only the OSACA analysis pickle (incompatible with the current osaca version).
        # Keep auto/kernel-marked.s so OSACA can skip the marking step and go straight to
        # analysis on the pre-marked icc-compiled assembly.
        find "${DISCOSTIC_KERNCRAFT_SCRATCH}" -maxdepth 5 -name "OSACA" -type d -exec rm -rf {} + 2>/dev/null || true
        echo "[ECM pre-gen] Seeded Kerncraft scratch from shared cache (kernel-marked.s kept; OSACA pickle regenerated fresh)"
    fi

    echo "[ECM pre-gen] arch=${arch} ECM_core=${ECM_core} N=${DEFINE_N} -> ${BASE_ECM_CFG}"
    python3 ./kerncraftintegration/diskern.py \
        -m"./nodelevel/machine-files/${arch}.yml" \
        -c"${ECM_core}" \
        -o"${BASE_ECM_CFG}" \
        -extras"${ECM_core}+0" \
        -systemNo0 \
        -p"${pmodel}" \
        -P"${cpred}" \
        --compiler-flags="${compiler_flags_raw}" \
        -D N "${DEFINE_N}" \
        -D s 55 \
        nodelevel/kernels/stream-triad.c 2>&1 || true

    if [ ! -f "${BASE_ECM_CFG}" ]; then
        echo "[ECM pre-gen] ERROR: diskern.py did not write ${BASE_ECM_CFG}" >&2
        return 1
    fi

    # Copy base cfg to all per-rank files so discostic can read them even if its
    # internal Kerncraft call fails (discostic reads FILENAME<rank>.cfg per rank).
    local max_rank="${TC_PROCNUMBERS}"
    for num in $(seq 0 "${max_rank}"); do
        cp "${BASE_ECM_CFG}" "${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR:-}${DISCOSTIC_ECM_SUBDIR:+/}${FILENAME}${num}.cfg"
    done
    echo "[ECM pre-gen] Wrote $(( max_rank + 1 )) per-rank cfg files"
    echo "[ECM pre-gen] Base cfg content:"
    cat "${BASE_ECM_CFG}" || true
}
_pre_generate_ecm_cfg

source "${TC_ROOT}/shared/scripts/discostic_mpi_env.sh"
discostic_srun | tee "${TC_SIM_OUT}/slurm-discostic-pass1.out"
echo "[POST-PASS1] Rank 0 cfg content:"
cat "${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR:-}${DISCOSTIC_ECM_SUBDIR:+/}${FILENAME}0.cfg" 2>/dev/null \
    || echo "[POST-PASS1] Rank 0 cfg missing"

if [ "${PART2}" != "0" ]; then
    rm -f "${ROOT}/nodelevel/configs/${FILENAME}"*.cfg 2>/dev/null || true
    # Re-copy per-rank files for pass2 (discostic will try to overwrite them from its
    # internal Kerncraft call, but if that fails the pre-generated files remain valid).
    if [ -f "${BASE_ECM_CFG}" ]; then
        for num in $(seq 0 "${TC_PROCNUMBERS}"); do
            cp "${BASE_ECM_CFG}" "${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR:-}${DISCOSTIC_ECM_SUBDIR:+/}${FILENAME}${num}.cfg"
        done
    fi
    discostic_srun | tee "${TC_SIM_OUT}/slurm-discostic-pass2.out"
fi

CSV=$(awk -F= '/^filename/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
DMS=$(awk -F= '/^chromevizfilename/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
[ -f "${ROOT}/${CSV}" ] && mv "${ROOT}/${CSV}" "${TC_SIM_OUT}/"
[ -f "${ROOT}/${DMS}.dms" ] && mv "${ROOT}/${DMS}.dms" "${TC_SIM_OUT}/" || true
[ -f "${ROOT}/${DMS}.avt" ] && mv "${ROOT}/${DMS}.avt" "${TC_SIM_OUT}/" || true

# Modeled SEND/RECV event log → ITAC STF (required when chromevizfilename is set).
if [ -n "${DMS}" ]; then
    if [ ! -f "${TC_SIM_OUT}/${DMS}.avt" ]; then
        echo "ERROR: missing ${TC_SIM_OUT}/${DMS}.avt (chromevizfilename=${DMS})" >&2
        exit 1
    fi
    echo "===== ITAC STF conversion (${DMS}.avt → ${DMS}.single.stf) ====="
    _stf_cmd=(bash "${ROOT}/visualization/convert_itac_stf.sh"
        "${TC_SIM_OUT}/${DMS}.avt"
        "${TC_SIM_OUT}/${DMS}.single.stf")
    # Cap Stage 5 conversion so otf2-to-stf cannot eat the remaining walltime
    # (large 4/8-node traces may abort; scoring still uses csv/dms/pass-2).
    case "${TC_STAGE_DIR}" in
        *stage5-scaling*) _stf_cmd=(timeout 180 "${_stf_cmd[@]}");;
    esac
    if "${_stf_cmd[@]}" && [ -f "${TC_SIM_OUT}/${DMS}.single.stf" ]; then
        echo "STF ready: ${TC_SIM_OUT}/${DMS}.single.stf"
    else
        case "${TC_STAGE_DIR}" in
            *stage5-scaling*)
                echo "WARNING: STF conversion failed; keeping ${DMS}.avt (sim results are valid)" >&2
                ;;
            *)
                echo "ERROR: STF conversion finished but ${DMS}.single.stf missing" >&2
                exit 1
                ;;
        esac
    fi
fi

[ -f "${BASE_ECM_CFG}" ] && cp "${BASE_ECM_CFG}" "${TC_SIM_OUT}/STREAM_FILE_ecm.cfg" || true
