#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Shared DisCostiC simulation job workflow for HEAT_FILE (Kerncraft heat.c).

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

KERN_DEFAULT="${ROOT}/nodelevel/kernels/.heat.c_kerncraft"
_restore_kerncraft_scratch() {
    if [ -n "${DISCOSTIC_KERNCRAFT_SCRATCH:-}" ] && [ -L "${KERN_DEFAULT}" ]; then
        rm -f "${KERN_DEFAULT}"
        if [ -d "${KERN_DEFAULT}.preserved-shared" ]; then
            mv "${KERN_DEFAULT}.preserved-shared" "${KERN_DEFAULT}"
        fi
    fi
}
_restore_discostic_repo() {
    git -C "${ROOT}" checkout -- config.cfg test/P2P.hpp 2>/dev/null || true
    git -C "${ROOT}" checkout -- nodelevel/configs/HEAT_FILE.cfg 2>/dev/null || true
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
# Always rebuild after linking HEAT_FILE.hpp. Reusing a STREAM-built binary
# yields zero COMP / wrong sends.
[ ! -d "${ROOT}/build" ] && cmake -S "${ROOT}" -B "${ROOT}/build"
make -C "${ROOT}/build" discostic
if [ ! -x "${ROOT}/discostic" ] && [ -x "${ROOT}/build/discostic" ]; then
    cp "${ROOT}/build/discostic" "${ROOT}/discostic"
fi
if [ ! -x "${ROOT}/discostic" ]; then
    echo "ERROR: discostic binary missing after build" >&2
    exit 1
fi

_pre_generate_ecm_cfg() {
    local arch pmodel cpred compiler_flags_raw
    local nproc_cfg tpn dim_x_cfg dim_y_cfg dim_y_local ECM_core cores_per_socket socket_scale

    arch=$(awk -F= '/^micro_architecture/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    pmodel=$(awk -F= '/^pmodel/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    cpred=$(awk -F= '/^cache_predictor/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    compiler_flags_raw=$(awk '/^compiler_flags/ {sub(/^compiler_flags *=[ \t]*/,""); gsub(/#.*/,""); gsub(/[[:space:]]+$/,""); print; exit}' config.cfg)
    nproc_cfg=$(awk -F= '/^number_of_processes/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    tpn=$(awk -F= '/^task_per_node/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    dim_x_cfg=$(awk -F= '/^dim_x / {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
    dim_y_cfg=$(awk -F= '/^dim_y / {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)

    cores_per_socket=$(grep -m1 "^cores per socket" "${ROOT}/nodelevel/machine-files/${arch}.yml" 2>/dev/null | awk '{print $NF}')
    [ -z "${cores_per_socket}" ] && cores_per_socket="${tpn}"

    # Match test/HEAT_FILE.hpp: dim_y_local = dim_y / primary_processes, then
    # if tpn != 1 multiply by min(tpn, cores_per_socket).
    dim_y_local=$(( dim_y_cfg / nproc_cfg ))
    ECM_core=1
    if [ "${tpn}" -ne 1 ]; then
        if [ "${tpn}" -lt "${cores_per_socket}" ]; then
            socket_scale="${tpn}"
        else
            socket_scale="${cores_per_socket}"
        fi
        dim_y_local=$(( dim_y_local * socket_scale ))
        ECM_core="${socket_scale}"
    fi

    if [ -n "${DISCOSTIC_KERNCRAFT_SCRATCH:-}" ] && [ -d "${KERN_DEFAULT}.preserved-shared" ]; then
        cp -rp "${KERN_DEFAULT}.preserved-shared/." "${DISCOSTIC_KERNCRAFT_SCRATCH}/"
        find "${DISCOSTIC_KERNCRAFT_SCRATCH}" -maxdepth 5 -name "OSACA" -type d -exec rm -rf {} + 2>/dev/null || true
        echo "[ECM pre-gen] Seeded Kerncraft scratch from shared cache (kernel-marked.s kept; OSACA pickle regenerated fresh)"
    fi

    echo "[ECM pre-gen] arch=${arch} ECM_core=${ECM_core} dim_x=${dim_x_cfg} dim_y=${dim_y_local} -> ${BASE_ECM_CFG}"
    python3 ./kerncraftintegration/diskern.py \
        -m"./nodelevel/machine-files/${arch}.yml" \
        -c"${ECM_core}" \
        -o"${BASE_ECM_CFG}" \
        -extras"${ECM_core}+0" \
        -systemNo0 \
        -p"${pmodel}" \
        -P"${cpred}" \
        --compiler-flags="${compiler_flags_raw}" \
        -D dim_x "${dim_x_cfg}" \
        -D dim_y "${dim_y_local}" \
        -D s 1 \
        nodelevel/kernels/heat.c 2>&1 || true

    if [ ! -f "${BASE_ECM_CFG}" ]; then
        echo "[ECM pre-gen] ERROR: diskern.py did not write ${BASE_ECM_CFG}" >&2
        return 1
    fi

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

if [ -n "${DMS}" ] && [ -f "${TC_SIM_OUT}/${DMS}.avt" ]; then
    echo "===== ITAC STF conversion (${DMS}.avt → ${DMS}.single.stf) ====="
    if bash "${ROOT}/visualization/convert_itac_stf.sh" \
        "${TC_SIM_OUT}/${DMS}.avt" \
        "${TC_SIM_OUT}/${DMS}.single.stf" \
        && [ -f "${TC_SIM_OUT}/${DMS}.single.stf" ]; then
        echo "STF ready: ${TC_SIM_OUT}/${DMS}.single.stf"
    else
        # otf2-to-stf aborts on large multi-node traces; Stage 5 walls still count.
        echo "WARNING: STF conversion failed; keeping ${DMS}.avt (sim results are valid)" >&2
    fi
fi

[ -f "${BASE_ECM_CFG}" ] && cp "${BASE_ECM_CFG}" "${TC_SIM_OUT}/HEAT_FILE_ecm.cfg" || true
