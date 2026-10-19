#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 4 idlewave DisCostiC simulation (icx-idlewave).

#SBATCH --job-name=PS_s4_icx_idlewave
#SBATCH --output=stage4-idlewave/results/simulated/icx/slurm.out
#SBATCH --error=stage4-idlewave/results/simulated/icx/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:14:00
#SBATCH --export=NONE
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_ROOT:-${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}}"
export TC_STAGE_DIR="${REPO_ROOT}/stage4-idlewave"
export TC_PROCNUMBERS="72"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs/icx-idlewave.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated/icx"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
