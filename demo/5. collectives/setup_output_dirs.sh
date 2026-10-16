#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Create SLURM output/error parent directories required before job submission.

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p \
  "${SCRIPT_DIR}/stage1-collective-cost/results/measured/bcast-binomial" \
  "${SCRIPT_DIR}/stage1-collective-cost/results/measured/bcast-ring" \
  "${SCRIPT_DIR}/stage1-collective-cost/results/measured/reduce-binomial" \
  "${SCRIPT_DIR}/stage1-collective-cost/results/measured/allreduce-recdbl" \
  "${SCRIPT_DIR}/stage1-collective-cost/results/simulated/bcast-binomial" \
  "${SCRIPT_DIR}/stage1-collective-cost/results/simulated/bcast-ring" \
  "${SCRIPT_DIR}/stage1-collective-cost/results/simulated/reduce-binomial" \
  "${SCRIPT_DIR}/stage1-collective-cost/results/simulated/allreduce-recdbl" \
  "${SCRIPT_DIR}/stage1-collective-cost/figures" \
  "${SCRIPT_DIR}/stage2-traces/results/measured-trace/icx" \
  "${SCRIPT_DIR}/stage2-traces/results/simulated/icx" \
  "${SCRIPT_DIR}/job_configs"

echo "All SLURM output directories created under: ${SCRIPT_DIR}"
