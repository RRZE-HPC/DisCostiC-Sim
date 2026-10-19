#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Pre-build measured pisolver-triad-bench binary.

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export MEASURED_BENCH_FORCE=1
bash "${SCRIPT_DIR}/build_benchmark.sh"
