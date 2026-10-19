#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
#
# Purpose: SLURM job: Stage 5 measured HEAT Jacobi 2D (weak-icx-1node).

#SBATCH --job-name=HEAT_s5_meas_weak_icx_1node
#SBATCH --output=stage5-scaling/results/measured/weak-icx-1node/slurm.out
#SBATCH --error=stage5-scaling/results/measured/weak-icx-1node/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:16:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage5-scaling/results/measured"
CASE_OUT="${OUT}/weak-icx-1node"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
measured_benchmark_prepare
TSV="${OUT}/stage5-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
OUTFILE="${CASE_OUT}/weak-icx-1node.out"
measured_srun_to_file 72 92160 16384 5000 "${OUTFILE}"
wall=$(grep '^wall_seconds=' "${OUTFILE}" | tail -1 | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
echo -e "weak-icx-1node\t92160\t16384\t5000\t72\t${wall}" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "Stage 5 measured weak-icx-1node done: wall=${wall}s"
