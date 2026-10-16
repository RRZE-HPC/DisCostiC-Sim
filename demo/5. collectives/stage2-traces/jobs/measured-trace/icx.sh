#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 2 measured IMB Bcast binomial with Intel Trace Collector (.stf).
#          Separate from Stage 1 scoring IMB so ITAC overhead does not change t_min/t_max.

#SBATCH --job-name=COLL_s2_mtr_icx
#SBATCH --output=stage2-traces/results/measured-trace/icx/slurm.out
#SBATCH --error=stage2-traces/results/measured-trace/icx/slurm.err
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
OUT="${REPO_ROOT}/stage2-traces/results/measured-trace"
CASE_OUT="${OUT}/icx"
mkdir -p "${CASE_OUT}"
module load intel/2023.2.1 intelmpi/2021.10.0 2>/dev/null \
    || module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null \
    || module load intel/2025.2.0 intelmpi/2021.16.0
module load intel-itac/2022.4.0 2>/dev/null || module load intel-itac 2>/dev/null || module load itac 2>/dev/null || true
source "${REPO_ROOT}/shared/scripts/imb_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
imb_clear_adjust
export I_MPI_ADJUST_BCAST=1
imb_srun_traced 72 Bcast "${CASE_OUT}/imb-trace.out" "bcast-binomial-measured"
echo "Stage 2 measured-trace ICX done: ${CASE_OUT}/bcast-binomial-measured.single.stf"
