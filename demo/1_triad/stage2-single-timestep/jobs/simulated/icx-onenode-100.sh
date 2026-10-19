#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Optional Stage 2 test — STREAM Triad DisCostiC sim, 1 node × 72 ranks (nop=72 → srun -n 73).
#          Not part of run_job_chain.py. Submit by hand if you want this case.

#SBATCH --job-name=TC_s2_sim_onenode
#SBATCH --output=stage2-single-timestep/results/simulated/icx-onenode-100/slurm.out
#SBATCH --error=stage2-single-timestep/results/simulated/icx-onenode-100/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --exclusive
#SBATCH --mem=60G
#SBATCH --time=00:35:00
#SBATCH --constraint=hwperf
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_ROOT:-${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}}"
export TC_STAGE_DIR="${REPO_ROOT}/stage2-single-timestep"
export TC_PROCNUMBERS="73"
export TC_CONFIG_FILE="${TC_STAGE_DIR}/configs/icx-onenode-100.cfg"
export TC_SIM_OUT="${TC_STAGE_DIR}/results/simulated/icx-onenode-100"
export DISCOSTIC_ECM_SUBDIR="job_${SLURM_JOB_ID:-local}"
export DISCOSTIC_CPU_FREQ=2400000-2400000:performance
export DISCOSTIC_SRUN_EXTRA="--overcommit"
mkdir -p "${TC_SIM_OUT}"
cd "${ROOT}"
source "${REPO_ROOT}/shared/scripts/run_discostic_common.sh"
