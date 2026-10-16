#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Build O3 and O1 variants of the measured STREAM Triad MPI benchmark.

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export MEASURED_BENCH_FORCE=1
MEASURED_BENCH_OPT=O3 bash "${SCRIPT_DIR}/build_benchmark.sh"
MEASURED_BENCH_OPT=O1 bash "${SCRIPT_DIR}/build_benchmark.sh"
