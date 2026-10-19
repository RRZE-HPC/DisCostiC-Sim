#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 4 measured STREAM Triad on full SPR node (idlewave baseline).

#SBATCH --job-name=TC_s4_meas_spr
#SBATCH --output=stage4-idlewave/results/measured/spr/slurm.out
#SBATCH --error=stage4-idlewave/results/measured/spr/slurm.err
#SBATCH --partition=spr1tb
#SBATCH --nodes=1
#SBATCH --ntasks=104
#SBATCH --ntasks-per-node=104
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:10:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2000000-2000000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage4-idlewave/results/measured"
CASE_OUT="${OUT}/spr"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2000000-2000000:performance
measured_benchmark_prepare
TSV="${OUT}/stage4-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
OUTFILE="${CASE_OUT}/spr.out"
# Match stage4 idlewave config scale (N / timesteps) on full dual-socket SPR.
N_TOTAL=2000000000
NTIMES=45
NPROCS=104
measured_srun_to_file "${NPROCS}" "${N_TOTAL}" "${NTIMES}" "${OUTFILE}"
line=$(grep '^wall_seconds=' "${OUTFILE}" | tail -1)
wall=$(echo "${line}" | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
bw=$(echo "${line}" | sed -n 's/.*bw_gb_s=\([^ ]*\).*/\1/p')
gflops=$(echo "${line}" | sed -n 's/.*gflops=\([^ ]*\).*/\1/p')
chk=$(echo "${line}" | sed -n 's/.*check=\([^ ]*\).*/\1/p')
echo -e "spr\t${N_TOTAL}\t${NTIMES}\t${NPROCS}\t${wall}\t${bw}\t${gflops}\t${chk}" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "Stage 4 measured SPR done: wall=${wall}s gflops=${gflops}"
