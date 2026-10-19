#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 3 DisCostiC simulation for config icx-tpn1-100.

#SBATCH --job-name=TC_s3_icx-tpn1-100
#SBATCH --output=stage3-multi-timestep/results/simulated/icx-tpn1-100/slurm.out
#SBATCH --error=stage3-multi-timestep/results/simulated/icx-tpn1-100/slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=36
#SBATCH --ntasks=36
#SBATCH --mem=10G
#SBATCH --time=00:15:00
#SBATCH --export=NONE
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_ROOT:-${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}}"
export TC_STAGE_DIR="${REPO_ROOT}/stage3-multi-timestep"
export TC_PROCNUMBERS="36"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs/icx-tpn1-100.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated/icx-tpn1-100"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
