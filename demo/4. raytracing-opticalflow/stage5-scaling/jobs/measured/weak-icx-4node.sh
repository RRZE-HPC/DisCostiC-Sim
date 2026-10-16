#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: measured RT+OF (weak-icx-4node).

#SBATCH --job-name=OF_s5m_w4n
#SBATCH --output=stage5-scaling/results/measured/weak-icx-4node/slurm.out
#SBATCH --error=stage5-scaling/results/measured/weak-icx-4node/slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=4
#SBATCH --ntasks=288
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=200G
#SBATCH --time=00:48:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage5-scaling/results/measured"
CASE_OUT="${REPO_ROOT}/stage5-scaling/results/measured/weak-icx-4node"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
measured_benchmark_prepare
TSV="${OUT}/stage5-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
OUTFILE="${CASE_OUT}/weak-icx-4node.out"
measured_srun_to_file 288 "11 2 10 0.00001 4 100 0 0.625 0.0" "${OUTFILE}"
read -r wall med nreps < <(measured_parse_wall_seconds "${OUTFILE}")
echo -e "weak-icx-4node\t11\t2\t4\t${nreps}\t288\t${wall}\t${med}\tok" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "Measured weak-icx-4node done: wall=${wall}s median_rep=${med}s reps=${nreps}"
