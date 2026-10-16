#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Build measured RT+OF solver binary.

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BENCH_DIR="${REPO_ROOT}/shared/rt-of-bench"
LOCKFILE="${BENCH_DIR}/.benchmark_build.lock"
OUT="${BENCH_DIR}/bin/ray-tracing-optical-flow-base"

module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
mkdir -p "${BENCH_DIR}/bin"

exec 201>"${LOCKFILE}"
flock -x 201
if [ "${MEASURED_BENCH_FORCE:-0}" = "1" ] || [ ! -x "${OUT}" ] || [ "${BENCH_DIR}/src/main.cpp" -nt "${OUT}" ]; then
    make -C "${BENCH_DIR}" all
else
    echo "Benchmark up to date -> ${OUT}"
fi
