#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: measured RT+OF (icx-tpn1-100).

#SBATCH --job-name=OF_s2_meas_tpn1
#SBATCH --output=stage2-single-timestep/results/measured/icx-tpn1-100/slurm.out
#SBATCH --error=stage2-single-timestep/results/measured/icx-tpn1-100/slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=36
#SBATCH --ntasks=36
#SBATCH --ntasks-per-node=1
#SBATCH --exclusive
#SBATCH --mem=10G
#SBATCH --time=00:10:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage2-single-timestep/results/measured"
CASE_OUT="${REPO_ROOT}/stage2-single-timestep/results/measured/icx-tpn1-100"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
measured_benchmark_prepare
TSV="${OUT}/stage2-measured-100.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
OUTFILE="${CASE_OUT}/icx-tpn1-100.out"
measured_srun_to_file 36 "9 2 10 0.00001 4 100 0 0.625 0.0" "${OUTFILE}"
read -r wall med nreps < <(measured_parse_wall_seconds "${OUTFILE}")
echo -e "icx-tpn1-100\t9\t2\t4\t${nreps}\t36\t${wall}\t${med}\tok" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "Measured icx-tpn1-100 done: wall=${wall}s median_rep=${med}s reps=${nreps}"
