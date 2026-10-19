#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 5 measured STREAM Triad (strong-icx-1node).

#SBATCH --job-name=TC_s5_meas_strong_icx_1node
#SBATCH --output=stage5-scaling/results/measured/strong-icx-1node/slurm.out
#SBATCH --error=stage5-scaling/results/measured/strong-icx-1node/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=01:06:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage5-scaling/results/measured"
CASE_OUT="${OUT}/strong-icx-1node"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
measured_benchmark_prepare
TSV="${OUT}/stage5-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
OUTFILE="${CASE_OUT}/strong-icx-1node.out"
N_TOTAL=5000000000
NTIMES=5000
NPROCS=72
measured_srun_to_file "${NPROCS}" "${N_TOTAL}" "${NTIMES}" "${OUTFILE}"
line=$(grep '^wall_seconds=' "${OUTFILE}" | tail -1)
wall=$(echo "${line}" | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
bw=$(echo "${line}" | sed -n 's/.*bw_gb_s=\([^ ]*\).*/\1/p')
gflops=$(echo "${line}" | sed -n 's/.*gflops=\([^ ]*\).*/\1/p')
chk=$(echo "${line}" | sed -n 's/.*check=\([^ ]*\).*/\1/p')
echo -e "strong-icx-1node\t${N_TOTAL}\t${NTIMES}\t${NPROCS}\t${wall}\t${bw}\t${gflops}\t${chk}" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "Stage 5 measured strong-icx-1node done: wall=${wall}s gflops=${gflops}"
