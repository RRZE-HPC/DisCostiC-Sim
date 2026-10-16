#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 2 DisCostiC simulation + STF, Bcast binomial.

#SBATCH --job-name=COLL_s2_bcast_bin
#SBATCH --output=stage2-traces/results/simulated/icx/slurm.out
#SBATCH --error=stage2-traces/results/simulated/icx/slurm.err
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
export TC_STAGE_DIR="${REPO_ROOT}/stage2-traces"
export TC_PROCNUMBERS="73"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs/bcast-binomial.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated/icx"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
