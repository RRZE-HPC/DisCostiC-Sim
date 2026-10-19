#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Create all SLURM output/error parent directories required before job submission.

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p \
  "${SCRIPT_DIR}/stage1-ecm-modeling/results/measured" \
  "${SCRIPT_DIR}/stage1-ecm-modeling/figures" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/measured" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/measured/icx-onenode-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/simulated/icx-socket-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/simulated/icx-tpn1-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/simulated/icx-onenode-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/results/simulated-penalty/icx-tpn1-100" \
  "${SCRIPT_DIR}/stage2-single-timestep/figures" \
  "${SCRIPT_DIR}/stage2-single-timestep/figures-penalty" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/measured" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-socket-100" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-socket-1000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-socket-10000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-tpn1-100" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-tpn1-1000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated/icx-tpn1-10000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated-penalty/icx-tpn1-100" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated-penalty/icx-tpn1-1000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/results/simulated-penalty/icx-tpn1-10000" \
  "${SCRIPT_DIR}/stage3-multi-timestep/figures" \
  "${SCRIPT_DIR}/stage3-multi-timestep/figures-penalty" \
  "${SCRIPT_DIR}/stage4-idlewave/results/measured/icx" \
  "${SCRIPT_DIR}/stage4-idlewave/results/measured-trace/icx" \
  "${SCRIPT_DIR}/stage4-idlewave/results/simulated/icx" \
  "${SCRIPT_DIR}/stage4-idlewave/results/simulated-penalty/icx" \
  "${SCRIPT_DIR}/stage4-idlewave/figures" \
  "${SCRIPT_DIR}/stage4-idlewave/figures-penalty" \
  "${SCRIPT_DIR}/stage5-scaling/figures" \
  "${SCRIPT_DIR}/stage5-scaling/figures-penalty" \
  "${SCRIPT_DIR}/stage5-scaling/results/measured" \
  "${SCRIPT_DIR}/job_configs"

for kind in strong weak; do
  for n in 1 2 4 8 16 32; do
    mkdir -p \
      "${SCRIPT_DIR}/stage5-scaling/results/measured/${kind}-icx-${n}node" \
      "${SCRIPT_DIR}/stage5-scaling/results/simulated/${kind}-icx-${n}node" \
      "${SCRIPT_DIR}/stage5-scaling/results/simulated-penalty/${kind}-icx-${n}node"
  done
done

echo "All SLURM output directories created under: ${SCRIPT_DIR}"
