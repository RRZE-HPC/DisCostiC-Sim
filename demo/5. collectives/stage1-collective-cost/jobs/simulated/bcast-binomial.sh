#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 1 DisCostiC simulation, Bcast binomial.

#SBATCH --job-name=COLL_s1_bcast_bin
#SBATCH --output=stage1-collective-cost/results/simulated/bcast-binomial/slurm.out
#SBATCH --error=stage1-collective-cost/results/simulated/bcast-binomial/slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=2
#SBATCH --ntasks=73
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:17:00
#SBATCH --export=NONE
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}"
export TC_STAGE_DIR="${REPO_ROOT}/stage1-collective-cost"
export TC_PROCNUMBERS="73"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs/bcast-binomial.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated/bcast-binomial"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
