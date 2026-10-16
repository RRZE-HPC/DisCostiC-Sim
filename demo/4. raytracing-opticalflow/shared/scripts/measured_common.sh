#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Helpers for measured RT+OF runs.

set -euo pipefail
: "${OUT:?OUT must be set}"

_MEASURED_SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
_MEASURED_REPO_ROOT="$(cd "${_MEASURED_SCRIPT_DIR}/../.." && pwd)"
BENCH_DIR="${_MEASURED_REPO_ROOT}/shared/rt-of-bench"
TSV_LOCK="${OUT}/.tsv.lock"
mkdir -p "${OUT}/tsv_fragments"
TSV_HEADER=$'config\tlevels\tss\tinner_steps\tnreps\tnprocs\twall_seconds\tmedian_rep_s\tcheck'

# Force Intel MPI async progress on socket-sized single-node jobs only
# (ntasks_per_node <= 36) so early Isend/Recv desync is reduced.
# Multinode (tpn1) has hung for hours with async; full-node 72-rank jobs
# inflated measured walls ~2x. Job scripts must load intelmpi first.
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
    local bin="${BENCH_DIR}/bin/ray-tracing-optical-flow-base"
    if [ ! -x "${bin}" ]; then
        echo "ERROR: benchmark binary missing: ${bin}" >&2
        return 1
    fi
    export MEASURED_BENCH_BIN="${bin}"
    export MEASURED_BENCH_DIR="${BENCH_DIR}"
}

# Parse Rank,NITER,TimePerReps,Performance CSV (one row per repetition).
# Wall matches DisCostiC max rank completion: max over ranks of sum(TimePerReps).
# Do not sum per-step maxima — that stacks MPI waits from different ranks that
# overlap in wall clock when there is no barrier between timesteps.
measured_parse_wall_seconds() {
    local logfile=$1
    python3 - "$logfile" <<'PY'
import re, sys, statistics
from collections import defaultdict
path = sys.argv[1]
by_rank = defaultdict(list)
with open(path, errors="replace") as f:
    for line in f:
        m = re.match(r"^(\d+),(\d+),([0-9.eE+-]+),", line.strip())
        if m:
            by_rank[int(m.group(1))].append((int(m.group(2)), float(m.group(3))))
if not by_rank:
    sys.stderr.write("ERROR: no timing rows in %s\n" % path)
    sys.exit(1)
rank_sums = {r: sum(w for _, w in recs) for r, recs in by_rank.items()}
slow = max(rank_sums, key=rank_sums.get)
steps = [w for _, w in sorted(by_rank[slow])]
print("%.6f %.6f %d" % (rank_sums[slow], statistics.median(steps), len(steps)))
PY
}

# Args: ntasks "CLA args..." outfile
measured_srun_to_file() {
    local ntasks=$1
    local cla=$2
    local out=$3
    local bin="${MEASURED_BENCH_BIN}"
    local work
    work="$(mktemp -d "${OUT}/run.XXXXXX")"
    cp "${bin}" "${work}/ray-tracing-optical-flow-base"
    mkdir -p "$(dirname "${out}")"
    set +o pipefail
    (
        cd "${work}"
        # shellcheck disable=SC2086
        srun \
            ${MEASURED_CPU_FREQ:+--cpu-freq="${MEASURED_CPU_FREQ}"} \
            ${MEASURED_CPU_BIND:+--cpu-bind="${MEASURED_CPU_BIND}"} \
            ${MEASURED_SRUN_EXTRA:-} \
            -n "${ntasks}" \
            ./ray-tracing-optical-flow-base ${cla}
    ) 2>&1 | tee "${out}"
    local rc=${PIPESTATUS[0]}
    set -o pipefail
    rm -rf "${work}"
    if [ "${rc}" -ne 0 ]; then
        echo "ERROR: srun failed (exit ${rc})" >&2
        return "${rc}"
    fi
    if ! grep -qE '^[0-9]+,[0-9]+,' "${out}"; then
        echo "ERROR: missing timing CSV rows in ${out}" >&2
        return 1
    fi
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
                fi
                echo "STF ready: ${dest}"
                return 0
            fi
        done
    done
    return 1
}

# Args: ntasks "CLA args..." outfile stf_stem
# Writes Intel Trace Collector STF next to logfile. Does NOT append the scoring TSV.
measured_srun_traced() {
    local ntasks=$1
    local cla=$2
    local out=$3
    local stf_stem=$4
    local bin="${MEASURED_BENCH_BIN}"
    local prefix libvt work
    mkdir -p "$(dirname "${out}")"
    prefix="$(cd "$(dirname "${out}")" && pwd)"
    measured_load_itac
    export VT_LOGFILE_PREFIX="${prefix}"
    export VT_LOGFILE_NAME="${stf_stem}"
    export VT_LOGFILE_FORMAT=SINGLESTF
    export VT_FLUSH_PREFIX="${prefix}"
    echo "ITAC: VT_LOGFILE_PREFIX=${prefix} VT_LOGFILE_NAME=${stf_stem} VT_ROOT=${VT_ROOT:-unset}"
    work="$(mktemp -d "${OUT}/run.XXXXXX")"
    cp "${bin}" "${work}/ray-tracing-optical-flow-base"
    set +o pipefail
    libvt="$(measured_find_libvt || true)"
    if [ -n "${libvt}" ]; then
        echo "ITAC: LD_PRELOAD=${libvt}"
        (
            cd "${work}"
            # shellcheck disable=SC2086
            LD_PRELOAD="${libvt}${LD_PRELOAD:+:${LD_PRELOAD}}" srun \
                ${MEASURED_CPU_FREQ:+--cpu-freq="${MEASURED_CPU_FREQ}"} \
                ${MEASURED_CPU_BIND:+--cpu-bind="${MEASURED_CPU_BIND}"} \
                ${MEASURED_SRUN_EXTRA:-} \
                -n "${ntasks}" \
                ./ray-tracing-optical-flow-base ${cla}
        ) 2>&1 | tee "${out}"
    elif command -v mpirun >/dev/null 2>&1; then
        echo "ITAC: mpirun -trace (libVT.so not found under VT_ROOT)"
        (
            cd "${work}"
            # shellcheck disable=SC2086
            mpirun -trace -n "${ntasks}" ./ray-tracing-optical-flow-base ${cla}
        ) 2>&1 | tee "${out}"
    else
        echo "ERROR: Intel Trace Collector not found (module load intel-itac / itac)" >&2
        set -o pipefail
        rm -rf "${work}"
        return 1
    fi
    local rc=${PIPESTATUS[0]}
    set -o pipefail
    if [ "${rc}" -ne 0 ]; then
        echo "ERROR: traced run failed (exit ${rc})" >&2
        rm -rf "${work}"
        return "${rc}"
    fi
    if _measured_harvest_stf "${prefix}" "${stf_stem}" "ray-tracing-optical-flow-base" "${work}" "${MEASURED_BENCH_DIR:-}"; then
        rm -rf "${work}"
        return 0
    fi
    echo "ERROR: ITAC did not write an STF under ${prefix} (stem=${stf_stem})" >&2
    ls -la "${prefix}" >&2 || true
    ls -la "${work}" >&2 || true
    rm -rf "${work}"
    return 1
}
