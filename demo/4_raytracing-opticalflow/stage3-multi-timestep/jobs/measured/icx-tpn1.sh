#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Stage 3 measured RT+OF (icx-tpn1) for 100/1000/10000 reps.

#SBATCH --job-name=OF_s3_meas_icx_tpn1
#SBATCH --output=stage3-multi-timestep/results/measured/icx-tpn1/slurm.out
#SBATCH --error=stage3-multi-timestep/results/measured/icx-tpn1/slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=36
#SBATCH --ntasks=36
#SBATCH --ntasks-per-node=1
#SBATCH --exclusive
#SBATCH --mem=10G
#SBATCH --time=02:55:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage3-multi-timestep/results/measured"
CASE_OUT="${OUT}/icx-tpn1"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
measured_benchmark_prepare
TSV="${OUT}/stage3-measured.tsv"
for reps in 100 1000 10000; do
  CFG_CASE="icx-tpn1-${reps}"
  OUTFILE="${CASE_OUT}/${CFG_CASE}.out"
  FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}_${reps}.tsv"
  CLA="9 2 10 0.00001 4 ${reps} 0 0.625 0.0"
  measured_srun_to_file 36 "${CLA}" "${OUTFILE}"
  read -r wall med nreps < <(measured_parse_wall_seconds "${OUTFILE}")
  echo -e "${CFG_CASE}\t9\t2\t4\t${nreps}\t36\t${wall}\t${med}\tok" > "${FRAG}"
  measured_tsv_append "${TSV}" "${FRAG}"
  echo "Stage3 ${CFG_CASE} wall=${wall}s"
done
