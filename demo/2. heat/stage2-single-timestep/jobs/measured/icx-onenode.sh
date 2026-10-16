#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
#
# Purpose: Optional Stage 2 test — measured HEAT Jacobi 2D, 1 node × 72 ranks.
#          Not part of run_job_chain.py. Submit by hand if you want this case.

#SBATCH --job-name=HEAT_s2_meas_onenode
#SBATCH --output=stage2-single-timestep/results/measured/icx-onenode-100/slurm.out
#SBATCH --error=stage2-single-timestep/results/measured/icx-onenode-100/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --time=00:10:00
#SBATCH --mem=130G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage2-single-timestep/results/measured"
CASE_OUT="${OUT}/icx-onenode-100"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
export MEASURED_CPU_BIND="map_cpu:$(seq -s, 0 71)"
measured_benchmark_prepare
TSV="${OUT}/stage2-measured-100.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
out="${CASE_OUT}/icx-onenode-100.out"
measured_srun_to_file 72 92160 16384 100 "${out}"
cp "${out}" "${OUT}/icx-onenode-100.out"
wall=$(grep '^wall_seconds=' "${out}" | tail -1 | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
echo -e "icx-onenode-100\t92160\t16384\t100\t72\t${wall}" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "ICX onenode Stage 2 measured done wall=${wall}s nprocs=72"
