#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: DisCostiC simulation (OF_s5_s2n).

#SBATCH --job-name=OF_s5_s2n
#SBATCH --output=stage5-scaling/results/simulated/strong-icx-2node/slurm.out
#SBATCH --error=stage5-scaling/results/simulated/strong-icx-2node/slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=2
#SBATCH --ntasks=144
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:10:00
#SBATCH --export=NONE
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_ROOT:-${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}}"
export TC_STAGE_DIR="${REPO_ROOT}/stage5-scaling"
export TC_PROCNUMBERS="144"
export TC_CONFIG_FILE="${REPO_ROOT}/stage5-scaling/configs/strong-icx-2node.cfg"
export TC_SIM_OUT="${REPO_ROOT}/stage5-scaling/results/simulated/strong-icx-2node"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
