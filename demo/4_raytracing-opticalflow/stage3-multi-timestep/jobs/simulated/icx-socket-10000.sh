#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: DisCostiC simulation (OF_s3_icx_socket_10000).

#SBATCH --job-name=OF_s3_icx_socket_10000
#SBATCH --output=stage3-multi-timestep/results/simulated/icx-socket-10000/slurm.out
#SBATCH --error=stage3-multi-timestep/results/simulated/icx-socket-10000/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=36
#SBATCH --ntasks-per-node=36
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
export TC_CONFIG_FILE="${REPO_ROOT}/stage3-multi-timestep/configs/icx-socket-10000.cfg"
export TC_SIM_OUT="${REPO_ROOT}/stage3-multi-timestep/results/simulated/icx-socket-10000"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
