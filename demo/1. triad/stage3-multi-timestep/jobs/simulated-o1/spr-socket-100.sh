#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 3 DisCostiC simulation for config spr-socket-100 (O1 case study).

#SBATCH --job-name=TC_s3o1_spr-socket-100
#SBATCH --output=stage3-multi-timestep/results/simulated-o1/spr-socket-100/slurm.out
#SBATCH --error=stage3-multi-timestep/results/simulated-o1/spr-socket-100/slurm.err
#SBATCH --partition=spr1tb
#SBATCH --nodes=1
#SBATCH --ntasks=52
#SBATCH --mem=60G
#SBATCH --time=00:18:00
#SBATCH --export=NONE
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}"
export TC_STAGE_DIR="${REPO_ROOT}/stage3-multi-timestep"
export TC_PROCNUMBERS="52"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs-o1/spr-socket-100.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated-o1/spr-socket-100"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
