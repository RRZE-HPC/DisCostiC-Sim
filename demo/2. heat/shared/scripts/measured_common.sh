#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Shared helpers for measured Jacobi 2D heat SLURM jobs.

set -euo pipefail

: "${OUT:?OUT must be set}"

_MEASURED_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
_MEASURED_REPO_ROOT="$(cd "${_MEASURED_SCRIPT_DIR}/../.." && pwd)"
BENCH_DIR="${_MEASURED_REPO_ROOT}/shared/benchmarks"
TSV_LOCK="${OUT}/.tsv.lock"
mkdir -p "${OUT}/tsv_fragments"
TSV_HEADER=$'config\tdim_y\tdim_x\tntimes\tnprocs\twall_seconds'

# Force Intel MPI async progress on socket-sized single-node jobs only
# (ntasks_per_node <= 36). Leave off for tpn1 and full-node 72-rank jobs.
_nodes="${SLURM_JOB_NUM_NODES:-1}"
_tpn="${SLURM_NTASKS_PER_NODE:-${SLURM_NTASKS:-1}}"
if [ "${_nodes}" -eq 1 ] && [ "${_tpn}" -le 36 ]; then
    export I_MPI_ASYNC_PROGRESS=1
    export I_MPI_ASYNC_PROGRESS_THREADS=1
else
    unset I_MPI_ASYNC_PROGRESS I_MPI_ASYNC_PROGRESS_THREADS || true
fi
unset _nodes _tpn

measured_benchmark_prepare() {
    "${_MEASURED_REPO_ROOT}/shared/scripts/build_benchmark.sh"
    local bin="${BENCH_DIR}/heat"
    case "${MEASURED_BENCH_OPT:-O3}" in
        O1|o1) bin="${BENCH_DIR}/heat_O1" ;;
    esac
    if [ ! -x "${bin}" ]; then
        echo "ERROR: benchmark binary missing: ${bin}" >&2
        return 1
    fi
    cd "${BENCH_DIR}"
    export MEASURED_BENCH_BIN="${bin}"
    export MEASURED_BENCH_DIR="${BENCH_DIR}"
}

# Args: ntasks dim_y dim_x iters outfile
measured_srun_to_file() {
    local ntasks=$1 dim_y=$2 dim_x=$3 iters=$4 out=$5
    local bin="${MEASURED_BENCH_BIN:-./heat}"
    if [[ "${bin}" == /* ]]; then
        bin="./$(basename "${bin}")"
    fi
    mkdir -p "$(dirname "${out}")"
    set +o pipefail
    # shellcheck disable=SC2086
    srun \
        ${MEASURED_CPU_FREQ:+--cpu-freq="${MEASURED_CPU_FREQ}"} \
        ${MEASURED_CPU_BIND:+--cpu-bind="${MEASURED_CPU_BIND}"} \
        ${MEASURED_SRUN_EXTRA:-} \
        -n "${ntasks}" \
        "${bin}" "${dim_y}" "${dim_x}" "${iters}" 2>&1 | tee "${out}"
    local rc=${PIPESTATUS[0]}
    set -o pipefail
    if [ "${rc}" -ne 0 ]; then
        echo "ERROR: srun failed (exit ${rc}) for ntimes=${iters}" >&2
        return "${rc}"
    fi
    if ! grep -q '^wall_seconds=' "${out}"; then
        echo "ERROR: missing wall_seconds line in ${out}" >&2
        return 1
    fi
}

measured_load_itac() {
    # Fritz: intel-itac/2022.4.0 (same as visualization/convert_itac_stf.sh).
    # NHR docs also name the module "itac".
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

# Copy/rename any ITAC SINGLESTF/STF under search dirs to ${prefix}/${stf_stem}.single.stf
_measured_harvest_stf() {
    local prefix=$1 stf_stem=$2
    local bin_stem=$3
    shift 3
    local dir stf dest
    dest="${prefix}/${stf_stem}.single.stf"
    for dir in "${prefix}" "$@"; do
        [ -n "${dir}" ] && [ -d "${dir}" ] || continue
        for stf in \
            "${dir}/${stf_stem}.single.stf" \
            "${dir}/${stf_stem}.stf" \
            "${dir}/${stf_stem}" \
            "${dir}/${bin_stem}.single.stf" \
            "${dir}/${bin_stem}.stf" \
            "${dir}/${bin_stem}"; do
            if [ -f "${stf}" ] && [ "$(wc -c < "${stf}" | tr -d ' ')" -ge 50 ]; then
                if [ "${stf}" != "${dest}" ]; then
                    cp -f "${stf}" "${dest}"
                    if [ "${dir}" != "${prefix}" ]; then
                        mv -f "${dir}/${bin_stem}".stf* "${prefix}/" 2>/dev/null || true
                        if [ -f "${dir}/${bin_stem}" ]; then
                            mv -f "${dir}/${bin_stem}" "${prefix}/" 2>/dev/null || true
                        fi
                    fi
                fi
                echo "STF ready: ${dest}"
                return 0
            fi
        done
    done
    return 1
}

# Args: ntasks dim_y dim_x iters logfile stf_stem
# Writes Intel Trace Collector STF next to logfile. Does NOT append the scoring TSV.
measured_srun_traced() {
    local ntasks=$1 dim_y=$2 dim_x=$3 iters=$4 out=$5 stf_stem=$6
    local bin="${MEASURED_BENCH_BIN:-./heat}"
    local prefix libvt bin_stem
    if [[ "${bin}" == /* ]]; then
        bin="./$(basename "${bin}")"
    fi
    bin_stem="${bin#./}"
    mkdir -p "$(dirname "${out}")"
    prefix="$(cd "$(dirname "${out}")" && pwd)"
    measured_load_itac
    export VT_LOGFILE_PREFIX="${prefix}"
    export VT_LOGFILE_NAME="${stf_stem}"
    export VT_LOGFILE_FORMAT=SINGLESTF
    export VT_FLUSH_PREFIX="${prefix}"
    echo "ITAC: VT_LOGFILE_PREFIX=${prefix} VT_LOGFILE_NAME=${stf_stem} VT_ROOT=${VT_ROOT:-unset}"
    set +o pipefail
    libvt="$(measured_find_libvt || true)"
    if [ -n "${libvt}" ]; then
        # Same launcher as the scoring job (srun + cpu-freq); libVT.so is NHR's MPI tracer.
        echo "ITAC: LD_PRELOAD=${libvt}"
        # shellcheck disable=SC2086
        LD_PRELOAD="${libvt}${LD_PRELOAD:+:${LD_PRELOAD}}" srun \
            ${MEASURED_CPU_FREQ:+--cpu-freq="${MEASURED_CPU_FREQ}"} \
            ${MEASURED_CPU_BIND:+--cpu-bind="${MEASURED_CPU_BIND}"} \
            ${MEASURED_SRUN_EXTRA:-} \
            -n "${ntasks}" \
            "${bin}" "${dim_y}" "${dim_x}" "${iters}" 2>&1 | tee "${out}"
    elif command -v mpirun >/dev/null 2>&1; then
        # NHR recipe: mpirun -trace (https://doc.nhr.fau.de/sdt/itac-tracing/)
        echo "ITAC: mpirun -trace (libVT.so not found under VT_ROOT)"
        mpirun -trace -n "${ntasks}" "${bin}" "${dim_y}" "${dim_x}" "${iters}" 2>&1 | tee "${out}"
    else
        echo "ERROR: Intel Trace Collector not found (module load intel-itac / itac)" >&2
        set -o pipefail
        return 1
    fi
    local rc=${PIPESTATUS[0]}
    set -o pipefail
    if [ "${rc}" -ne 0 ]; then
        echo "ERROR: traced run failed (exit ${rc}) for ntimes=${iters}" >&2
        return "${rc}"
    fi
    if _measured_harvest_stf "${prefix}" "${stf_stem}" "${bin_stem}" "${MEASURED_BENCH_DIR:-}"; then
        return 0
    fi
    echo "ERROR: ITAC did not write an STF under ${prefix} (stem=${stf_stem})" >&2
    ls -la "${prefix}" >&2 || true
    ls -la "${MEASURED_BENCH_DIR:-.}"/*.stf* >&2 || true
    return 1
}

measured_tsv_append() {
    local tsv=$1 frag=$2
    exec 9>"${TSV_LOCK}"
    flock -x 9
    if [ ! -f "${tsv}" ]; then
        echo -e "${TSV_HEADER}" > "${tsv}"
    fi
    cat "${frag}" >> "${tsv}"
}
