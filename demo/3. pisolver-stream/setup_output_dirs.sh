#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Create SLURM output directories (ICX / O3 only) matching streamtriad layout.

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p \
  "${SCRIPT_DIR}/job_configs" \
  "${SCRIPT_DIR}/stage1-ecm-modeling/results/measured" \
  "${SCRIPT_DIR}/stage1-ecm-modeling/figures" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/measured" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/measured/icx-socket-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/measured/icx-tpn1-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/measured/icx-onenode-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/simulated/icx-socket-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/simulated/icx-tpn1-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/simulated/icx-onenode-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/figures" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/measured" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-socket-100" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-socket-1000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-socket-10000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-tpn1-100" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-tpn1-1000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-tpn1-10000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/figures" \
  "${SCRIPT_DIR}/stage4-idlewave/figures" \
  "${SCRIPT_DIR}/stage4-idlewave/results/measured/icx" \
  "${SCRIPT_DIR}/stage4-idlewave/results/measured-trace/icx" \
  "${SCRIPT_DIR}/stage4-idlewave/results/simulated/icx" \
  "${SCRIPT_DIR}/stage5-scaling/figures" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/strong-icx-1node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/strong-icx-2node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/strong-icx-4node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/strong-icx-8node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/strong-icx-16node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/strong-icx-32node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/weak-icx-1node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/weak-icx-2node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/weak-icx-4node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/weak-icx-8node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/weak-icx-16node" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured/weak-icx-32node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/strong-icx-1node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/strong-icx-2node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/strong-icx-4node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/strong-icx-8node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/strong-icx-16node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/strong-icx-32node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/weak-icx-1node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/weak-icx-2node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/weak-icx-4node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/weak-icx-8node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/weak-icx-16node" \
  "${SCRIPT_DIR}/stage5-scaling/results/simulated/weak-icx-32node"

echo "All SLURM output directories created under: ${SCRIPT_DIR}"
