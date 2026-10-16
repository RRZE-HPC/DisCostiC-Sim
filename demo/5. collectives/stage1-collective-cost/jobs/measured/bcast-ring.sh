#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 1 measured IMB Bcast ring (I_MPI_ADJUST_BCAST=3).

#SBATCH --job-name=COLL_s1_imb_bring
#SBATCH --output=stage1-collective-cost/results/measured/bcast-ring/slurm.out
#SBATCH --error=stage1-collective-cost/results/measured/bcast-ring/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:20:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage1-collective-cost/results/measured"
CASE_OUT="${OUT}/bcast-ring"
mkdir -p "${CASE_OUT}"
module load intel/2023.2.1 intelmpi/2021.10.0 2>/dev/null \
    || module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null \
    || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/imb_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
imb_clear_adjust
export I_MPI_ADJUST_BCAST=3
imb_srun_to_file 72 Bcast "${CASE_OUT}/imb.out"
echo "Stage 1 IMB Bcast ring done: ${CASE_OUT}/imb.out"
