#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Build all measured RT+OF variants (currently base only).
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export MEASURED_BENCH_FORCE=1
bash "${SCRIPT_DIR}/build_benchmark.sh"
