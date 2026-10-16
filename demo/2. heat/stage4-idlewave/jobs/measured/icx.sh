#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
#
# Purpose: SLURM job: Stage 4 measured HEAT Jacobi 2D on full ICX node (idlewave baseline).

#SBATCH --job-name=HEAT_s4_meas_icx
#SBATCH --output=stage4-idlewave/results/measured/icx/slurm.out
#SBATCH --error=stage4-idlewave/results/measured/icx/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:10:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage4-idlewave/results/measured"
CASE_OUT="${OUT}/icx"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
# Match DisCostiC idlewave: 25 extra COMP on rank 4 at timestep 5.
export HEAT_IDLEWAVE_RANK=4
export HEAT_IDLEWAVE_STEP=5
export HEAT_IDLEWAVE_MULT=25
measured_benchmark_prepare
TSV="${OUT}/stage4-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
OUTFILE="${CASE_OUT}/icx.out"
measured_srun_to_file 72 92160 16384 45 "${OUTFILE}"
wall=$(grep '^wall_seconds=' "${OUTFILE}" | tail -1 | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
echo -e "icx\t92160\t16384\t45\t72\t${wall}" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "Stage 4 measured ICX done: wall=${wall}s"
