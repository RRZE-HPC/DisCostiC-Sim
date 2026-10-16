#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 5 scaling DisCostiC simulation (weak-spr-8node).

#SBATCH --job-name=TC_s5_weak_spr_8node
#SBATCH --output=stage5-scaling/results/simulated/weak-spr-8node/slurm.out
#SBATCH --error=stage5-scaling/results/simulated/weak-spr-8node/slurm.err
#SBATCH --partition=spr1tb
#SBATCH --nodes=8
#SBATCH --ntasks=832
#SBATCH --ntasks-per-node=104
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=01:00:00
#SBATCH --export=NONE
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}"
export TC_STAGE_DIR="${REPO_ROOT}/stage5-scaling"
export TC_PROCNUMBERS="832"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs/weak-spr-8node.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated/weak-spr-8node"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
