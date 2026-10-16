#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: DisCostiC sim workflow for PITRIADLBL (LBL DIVIDE + LBL STREAM).

set -euo pipefail
: "${TC_STAGE_DIR:?TC_STAGE_DIR must be set}"
: "${TC_PROCNUMBERS:?TC_PROCNUMBERS must be set}"
: "${TC_CONFIG_FILE:?TC_CONFIG_FILE must be set}"
: "${TC_SIM_OUT:?TC_SIM_OUT must be set}"

_COMMON_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TC_ROOT="$(cd "${_COMMON_SCRIPT_DIR}/../.." && pwd)"
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${TC_ROOT}/../.." && pwd)}"
LOCKFILE="${TC_ROOT}/.discostic_workspace.lock"
JOB_ID="${SLURM_JOB_ID:-local}"
JOB_CFG="${TC_ROOT}/job_configs/job_${JOB_ID}.cfg"

module load python git cmake likwid intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null \
    || module load python git cmake likwid intel/2025.2.0 intelmpi/2021.16.0

cd "${ROOT}"
mkdir -p "${TC_ROOT}/job_configs" "${TC_SIM_OUT}"
exec 200>"${LOCKFILE}"
flock -x 200
cp "${TC_CONFIG_FILE}" "${JOB_CFG}"
cp "${JOB_CFG}" config.cfg

FILENAME=$(awk -F= '/^benchmark_kernel/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
PART1=$(awk -F= '/^part1/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
PART2=$(awk -F= '/^part2/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)

if [ -n "${DISCOSTIC_ECM_SUBDIR:-}" ]; then
    mkdir -p "${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR}"
    PART1_STEM="${DISCOSTIC_ECM_SUBDIR}/${PART1}"
    PART2_STEM="${DISCOSTIC_ECM_SUBDIR}/${PART2}"
    sed -i "s|^part1 *=.*|part1 = ${PART1_STEM}|" config.cfg
    sed -i "s|^part2 *=.*|part2 = ${PART2_STEM}|" config.cfg
    PART1="${PART1_STEM}"
    PART2="${PART2_STEM}"
fi

_restore_discostic_repo() {
    git -C "${ROOT}" checkout -- config.cfg test/P2P.hpp 2>/dev/null || true
    if [ -n "${DISCOSTIC_ECM_SUBDIR:-}" ]; then
        rm -rf "${ROOT}/nodelevel/configs/${DISCOSTIC_ECM_SUBDIR}" 2>/dev/null || true
    fi
}
trap _restore_discostic_repo EXIT

for stem in "${PART1}" "${PART2}"; do
    base="$(basename "${stem}")"
    dir="$(dirname "${stem}")"
    if [ "${dir}" = "." ]; then
        rm -f "${ROOT}/nodelevel/configs/${base}"*.cfg 2>/dev/null || true
    else
        rm -f "${ROOT}/nodelevel/configs/${dir}/${base}"*.cfg 2>/dev/null || true
    fi
done

ln -sfn "${FILENAME}.hpp" "${ROOT}/test/P2P.hpp"
touch "${ROOT}/include/Grid.hpp"
[ ! -d "${ROOT}/build" ] && cmake -S "${ROOT}" -B "${ROOT}/build"
make -C "${ROOT}/build" discostic
if [ ! -x "${ROOT}/discostic" ] && [ -x "${ROOT}/build/discostic" ]; then
    cp "${ROOT}/build/discostic" "${ROOT}/discostic"
fi
if [ ! -x "${ROOT}/discostic" ]; then
    echo "ERROR: discostic binary missing after build" >&2
    exit 1
fi

_install_lbl_ecm() {
    local stem=$1 src_name=$2
    local src dst cfg_dir cfg_base num
    src="${ROOT}/nodelevel/configs/${src_name}.cfg"
    if [ ! -f "${src}" ]; then
        echo "[ECM pre-gen] ERROR: missing hand ECM ${src}" >&2
        return 1
    fi
    cfg_dir="$(dirname "${stem}")"
    cfg_base="$(basename "${stem}")"
    if [ "${cfg_dir}" = "." ]; then
        dst="${ROOT}/nodelevel/configs/${cfg_base}.cfg"
    else
        mkdir -p "${ROOT}/nodelevel/configs/${cfg_dir}"
        dst="${ROOT}/nodelevel/configs/${cfg_dir}/${cfg_base}.cfg"
    fi
    cp "${src}" "${dst}"
    for num in $(seq 0 "${TC_PROCNUMBERS}"); do
        if [ "${cfg_dir}" = "." ]; then
            cp "${dst}" "${ROOT}/nodelevel/configs/${cfg_base}${num}.cfg"
        else
            cp "${dst}" "${ROOT}/nodelevel/configs/${cfg_dir}/${cfg_base}${num}.cfg"
        fi
    done
    echo "[ECM pre-gen] Installed LBL ${src_name} -> ${dst}"
    cat "${dst}" || true
}

# part1 = DIVIDE (Pi), part2 = STREAM — both LBL hand ECM (PITRIADLBL)
_install_lbl_ecm "${PART1}" "DIVIDE"
_install_lbl_ecm "${PART2}" "STREAM"

source "${TC_ROOT}/shared/scripts/discostic_mpi_env.sh"
discostic_srun | tee "${TC_SIM_OUT}/slurm-discostic-pass1.out"

if [ "$(basename "${PART2}")" != "0" ] && [ "${PART2}" != "0" ]; then
    for stem in "${PART1}" "${PART2}"; do
        cfg_dir="$(dirname "${stem}")"
        cfg_base="$(basename "${stem}")"
        if [ "${cfg_dir}" = "." ]; then
            base_cfg="${ROOT}/nodelevel/configs/${cfg_base}.cfg"
            [ -f "${base_cfg}" ] || continue
            for num in $(seq 0 "${TC_PROCNUMBERS}"); do
                cp "${base_cfg}" "${ROOT}/nodelevel/configs/${cfg_base}${num}.cfg"
            done
        else
            base_cfg="${ROOT}/nodelevel/configs/${cfg_dir}/${cfg_base}.cfg"
            [ -f "${base_cfg}" ] || continue
            for num in $(seq 0 "${TC_PROCNUMBERS}"); do
                cp "${base_cfg}" "${ROOT}/nodelevel/configs/${cfg_dir}/${cfg_base}${num}.cfg"
            done
        fi
    done
    discostic_srun | tee "${TC_SIM_OUT}/slurm-discostic-pass2.out"
fi

CSV=$(awk -F= '/^filename/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
DMS=$(awk -F= '/^chromevizfilename/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg)
[ -f "${ROOT}/${CSV}" ] && mv "${ROOT}/${CSV}" "${TC_SIM_OUT}/"
[ -f "${ROOT}/${DMS}.dms" ] && mv "${ROOT}/${DMS}.dms" "${TC_SIM_OUT}/" || true
[ -f "${ROOT}/${DMS}.avt" ] && mv "${ROOT}/${DMS}.avt" "${TC_SIM_OUT}/" || true

# Modeled SEND/RECV event log → ITAC STF (required for all stage sim results).
if [ -n "${DMS}" ] && [ -f "${TC_SIM_OUT}/${DMS}.avt" ]; then
    echo "===== ITAC STF conversion (${DMS}.avt → ${DMS}.single.stf) ====="
    bash "${ROOT}/visualization/convert_itac_stf.sh" \
        "${TC_SIM_OUT}/${DMS}.avt" \
        "${TC_SIM_OUT}/${DMS}.single.stf"
    if [ ! -f "${TC_SIM_OUT}/${DMS}.single.stf" ]; then
        echo "ERROR: STF conversion finished but ${DMS}.single.stf missing" >&2
        exit 1
    fi
    echo "STF ready: ${TC_SIM_OUT}/${DMS}.single.stf"
fi

cfg_dir="$(dirname "${PART1}")"; cfg_base="$(basename "${PART1}")"
if [ "${cfg_dir}" = "." ]; then
    [ -f "${ROOT}/nodelevel/configs/${cfg_base}.cfg" ] && cp "${ROOT}/nodelevel/configs/${cfg_base}.cfg" "${TC_SIM_OUT}/DIVIDE_ecm.cfg" || true
else
    [ -f "${ROOT}/nodelevel/configs/${cfg_dir}/${cfg_base}.cfg" ] && cp "${ROOT}/nodelevel/configs/${cfg_dir}/${cfg_base}.cfg" "${TC_SIM_OUT}/DIVIDE_ecm.cfg" || true
fi
cfg_dir="$(dirname "${PART2}")"; cfg_base="$(basename "${PART2}")"
if [ "${cfg_dir}" = "." ]; then
    [ -f "${ROOT}/nodelevel/configs/${cfg_base}.cfg" ] && cp "${ROOT}/nodelevel/configs/${cfg_base}.cfg" "${TC_SIM_OUT}/STREAM_ecm.cfg" || true
else
    [ -f "${ROOT}/nodelevel/configs/${cfg_dir}/${cfg_base}.cfg" ] && cp "${ROOT}/nodelevel/configs/${cfg_dir}/${cfg_base}.cfg" "${TC_SIM_OUT}/STREAM_ecm.cfg" || true
fi
