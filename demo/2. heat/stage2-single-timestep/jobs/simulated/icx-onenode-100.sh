#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
#
# Purpose: Optional Stage 2 test — DisCostiC sim for icx-onenode-100 (1 node × 72 ranks).
#          Not part of run_job_chain.py. Submit by hand if you want this case.

#SBATCH --job-name=HEAT_s2_icx_onenode
#SBATCH --output=stage2-single-timestep/results/simulated/icx-onenode-100/slurm.out
#SBATCH --error=stage2-single-timestep/results/simulated/icx-onenode-100/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:10:00
#SBATCH --export=NONE
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}"
export TC_STAGE_DIR="${REPO_ROOT}/stage2-single-timestep"
export TC_PROCNUMBERS="72"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs/icx-onenode-100.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated/icx-onenode-100"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
