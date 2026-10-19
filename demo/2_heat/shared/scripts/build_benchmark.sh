#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Build the shared Jacobi 2D heat MPI benchmark (O3 or O1 / AVX-512) with flock protection.

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BENCH_DIR="${REPO_ROOT}/shared/benchmarks"
SRC="${BENCH_DIR}/heat.c"
LOCKFILE="${BENCH_DIR}/.benchmark_build.lock"
OPT="${MEASURED_BENCH_OPT:-O3}"
case "${OPT}" in
    O1|o1) OPT=O1; OUT="${BENCH_DIR}/heat_O1"; COPT="-O1" ;;
    *)     OPT=O3; OUT="${BENCH_DIR}/heat";    COPT="-O3" ;;
esac
# Compiler flags: -O{3|1} -fno-alias -xCORE-AVX512 -qopt-zmm-usage=high. -O1 is the case study.
CFLAGS="${COPT} -fno-alias -xCORE-AVX512 -qopt-zmm-usage=high"
STAGING="${BENCH_DIR}/.build/heat_${OPT}"

module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
mkdir -p "${BENCH_DIR}/.build"

exec 201>"${LOCKFILE}"
flock -x 201
if [ "${MEASURED_BENCH_FORCE:-0}" = "1" ] || [ ! -x "${OUT}" ] || [ "${SRC}" -nt "${OUT}" ]; then
    cd "${BENCH_DIR}"
    # shellcheck disable=SC2086
    if mpiicc ${CFLAGS} -o "${STAGING}" heat.c 2>/dev/null; then
        echo "Built with mpiicc ${CFLAGS} -> ${OUT}"
    else
        mpicc "${COPT}" -fno-alias -march=native -o "${STAGING}" heat.c
        echo "Built with mpicc fallback ${COPT} -> ${OUT}"
    fi
    mv -f "${STAGING}" "${OUT}"
    chmod +x "${OUT}"
else
    echo "Benchmark up to date (${OPT}) -> ${OUT}"
fi
