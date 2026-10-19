#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
#
# Purpose: SLURM job: Stage 3 DisCostiC simulation for config icx-socket-100.

#SBATCH --job-name=HEAT_s3_icx-socket_100
#SBATCH --output=stage3-multi-timestep/results/simulated/icx-socket-100/slurm.out
#SBATCH --error=stage3-multi-timestep/results/simulated/icx-socket-100/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=36
#SBATCH --exclusive
#SBATCH --mem=60G
#SBATCH --time=00:10:00
#SBATCH --export=NONE
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_ROOT:-${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}}"
export TC_STAGE_DIR="${REPO_ROOT}/stage3-multi-timestep"
export TC_PROCNUMBERS="36"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs/icx-socket-100.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated/icx-socket-100"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
