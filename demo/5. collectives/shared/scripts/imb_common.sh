#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Shared helpers for measured Intel MPI Benchmarks (IMB-MPI1) collective jobs.

set -euo pipefail

: "${OUT:?OUT must be set}"

_IMB_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
_IMB_REPO_ROOT="$(cd "${_IMB_SCRIPT_DIR}/../.." && pwd)"
SIZES_FILE="${_IMB_REPO_ROOT}/shared/sizes.txt"

if [ "${SLURM_JOB_NUM_NODES:-1}" -gt 1 ]; then
    export I_MPI_FABRICS="${I_MPI_FABRICS:-shm:ofi}"
    export I_MPI_OFI_PROVIDER="${I_MPI_OFI_PROVIDER:-verbs}"
    export FI_PROVIDER="${FI_PROVIDER:-verbs;ofi_rxm}"
else
    # One node: on-node shm, pin 0–71.
    export I_MPI_FABRICS="${I_MPI_FABRICS:-shm}"
fi

find_imb_mpi1() {
    local cand
    for cand in \
        "${I_MPI_ROOT:+${I_MPI_ROOT}/bin/IMB-MPI1}" \
        "${I_MPI_ROOT:+${I_MPI_ROOT}/linux/bin/IMB-MPI1}" \
        "${I_MPI_ROOT:+${I_MPI_ROOT}/intel64/bin/IMB-MPI1}" \
        "${I_MPI_ROOT:+${I_MPI_ROOT}/bin/intel64/IMB-MPI1}"; do
        if [ -n "${cand}" ] && [ -x "${cand}" ]; then
            echo "${cand}"
            return 0
        fi
    done
    if command -v IMB-MPI1 >/dev/null 2>&1; then
        command -v IMB-MPI1
        return 0
    fi
    echo "ERROR: IMB-MPI1 not found (I_MPI_ROOT=${I_MPI_ROOT:-unset}). Load intelmpi and retry." >&2
    return 1
}

imb_pin_env() {
    export I_MPI_PIN="${I_MPI_PIN:-1}"
    export I_MPI_PIN_DOMAIN="${I_MPI_PIN_DOMAIN:-core}"
    local tpn="${SLURM_NTASKS_PER_NODE:-72}"
    tpn="${tpn%%(*}"
    tpn="${tpn%%,*}"
    if [ -z "${I_MPI_PIN_PROCESSOR_LIST:-}" ]; then
        export I_MPI_PIN_PROCESSOR_LIST="0-$((tpn - 1))"
    fi
}

# Unset sibling collective ADJUST vars so only the intended algorithm is pinned.
imb_clear_adjust() {
    unset I_MPI_ADJUST_BCAST I_MPI_ADJUST_REDUCE I_MPI_ADJUST_ALLREDUCE \
        I_MPI_ADJUST_GATHER I_MPI_ADJUST_SCATTER I_MPI_ADJUST_ALLGATHER \
        I_MPI_ADJUST_ALLTOALL I_MPI_ADJUST_BARRIER || true
}

# Args: ntasks imb_op logfile
# Env: I_MPI_ADJUST_<OP>=<id> must already be set.
# IMB-MPI1 requires "-root_shift on|off" (bare -root_shift is a parse error).
# IMB_ROOT_SHIFT defaults to on (rooted Bcast/Reduce). Set to off for Allreduce.
imb_srun_to_file() {
    local ntasks=$1 imb_op=$2 out=$3
    local imb root_shift="${IMB_ROOT_SHIFT:-on}"
    imb="$(find_imb_mpi1)"
    mkdir -p "$(dirname "${out}")"
    if [ ! -f "${SIZES_FILE}" ]; then
        echo "ERROR: missing message-size list ${SIZES_FILE}" >&2
        return 1
    fi
    imb_pin_env
    echo "IMB: binary=${imb} op=${imb_op} sizes=${SIZES_FILE} root_shift=${root_shift}"
    echo "IMB: I_MPI_ADJUST_BCAST=${I_MPI_ADJUST_BCAST:-} I_MPI_ADJUST_REDUCE=${I_MPI_ADJUST_REDUCE:-} I_MPI_ADJUST_ALLREDUCE=${I_MPI_ADJUST_ALLREDUCE:-}"
    set +o pipefail
    # shellcheck disable=SC2086
    srun \
        ${MEASURED_CPU_FREQ:+--cpu-freq="${MEASURED_CPU_FREQ}"} \
        ${MEASURED_CPU_BIND:+--cpu-bind="${MEASURED_CPU_BIND}"} \
        ${MEASURED_SRUN_EXTRA:-} \
        -n "${ntasks}" \
        "${imb}" \
        -npmin "${ntasks}" \
        -iter 10000 \
        -iter_policy dynamic \
        -msglen "${SIZES_FILE}" \
        -root_shift "${root_shift}" \
        "${imb_op}" 2>&1 | tee "${out}"
    local rc=${PIPESTATUS[0]}
    set -o pipefail
    if [ "${rc}" -ne 0 ]; then
        echo "ERROR: IMB srun failed (exit ${rc}) for ${imb_op}" >&2
        return "${rc}"
    fi
    if ! grep -Eq '#bytes|t_min' "${out}"; then
        echo "ERROR: IMB output missing t_min table in ${out}" >&2
        return 1
    fi
}

measured_load_itac() {
    module load intel-itac/2022.4.0 2>/dev/null \
        || module load intel-itac 2>/dev/null \
        || module load itac 2>/dev/null \
        || true
}

measured_find_libvt() {
    local cand
    for cand in \
        "${VT_ROOT:+${VT_ROOT}/slib/libVT.so}" \
        "${VT_ROOT:+${VT_ROOT}/lib/libVT.so}" \
        "${VT_ROOT:+${VT_ROOT}/lib/intel64/libVT.so}" \
        "${VT_LIB:+${VT_LIB}/libVT.so}"; do
        if [ -n "${cand}" ] && [ -f "${cand}" ]; then
            echo "${cand}"
            return 0
        fi
    done
    return 1
}

_harvest_stf() {
    local prefix=$1 stf_stem=$2
    local dest="${prefix}/${stf_stem}.single.stf"
    local stf
    for stf in \
        "${prefix}/${stf_stem}.single.stf" \
        "${prefix}/${stf_stem}.stf" \
        "${prefix}/${stf_stem}"; do
        if [ -f "${stf}" ] && [ "$(wc -c < "${stf}" | tr -d ' ')" -ge 50 ]; then
            if [ "${stf}" != "${dest}" ]; then
                cp -f "${stf}" "${dest}"
            fi
            echo "STF ready: ${dest}"
            return 0
        fi
    done
    return 1
}

# Args: ntasks imb_op logfile stf_stem
# ITAC-traced IMB run. Does NOT write the scoring TSV.
imb_srun_traced() {
    local ntasks=$1 imb_op=$2 out=$3 stf_stem=$4
    local imb prefix libvt root_shift="${IMB_ROOT_SHIFT:-on}"
    imb="$(find_imb_mpi1)"
    mkdir -p "$(dirname "${out}")"
    prefix="$(cd "$(dirname "${out}")" && pwd)"
    measured_load_itac
    imb_pin_env
    export VT_LOGFILE_PREFIX="${prefix}"
    export VT_LOGFILE_NAME="${stf_stem}"
    export VT_LOGFILE_FORMAT=SINGLESTF
    export VT_FLUSH_PREFIX="${prefix}"
    echo "ITAC: VT_LOGFILE_PREFIX=${prefix} VT_LOGFILE_NAME=${stf_stem} VT_ROOT=${VT_ROOT:-unset}"
    set +o pipefail
    libvt="$(measured_find_libvt || true)"
    if [ -n "${libvt}" ]; then
        echo "ITAC: LD_PRELOAD=${libvt}"
        # shellcheck disable=SC2086
        LD_PRELOAD="${libvt}${LD_PRELOAD:+:${LD_PRELOAD}}" srun \
            ${MEASURED_CPU_FREQ:+--cpu-freq="${MEASURED_CPU_FREQ}"} \
            ${MEASURED_CPU_BIND:+--cpu-bind="${MEASURED_CPU_BIND}"} \
            ${MEASURED_SRUN_EXTRA:-} \
            -n "${ntasks}" \
            "${imb}" \
            -npmin "${ntasks}" \
            -iter 100 \
            -iter_policy dynamic \
            -msglen "${SIZES_FILE}" \
            -root_shift "${root_shift}" \
            "${imb_op}" 2>&1 | tee "${out}"
    elif command -v mpirun >/dev/null 2>&1; then
        echo "ITAC: mpirun -trace (libVT.so not found under VT_ROOT)"
        mpirun -trace -n "${ntasks}" "${imb}" \
            -npmin "${ntasks}" -iter 100 -iter_policy dynamic \
            -msglen "${SIZES_FILE}" -root_shift "${root_shift}" "${imb_op}" 2>&1 | tee "${out}"
    else
        echo "ERROR: Intel Trace Collector not found (module load intel-itac / itac)" >&2
        set -o pipefail
        return 1
    fi
    local rc=${PIPESTATUS[0]}
    set -o pipefail
    if [ "${rc}" -ne 0 ]; then
        echo "ERROR: traced IMB run failed (exit ${rc}) for ${imb_op}" >&2
        return "${rc}"
    fi
    if _harvest_stf "${prefix}" "${stf_stem}"; then
        return 0
    fi
    echo "ERROR: ITAC did not write an STF under ${prefix} (stem=${stf_stem})" >&2
    ls -la "${prefix}" >&2 || true
    return 1
}
