#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
#
# Purpose: SLURM job: Stage 5 measured Pi+STREAM (strong-icx-16node).

#SBATCH --job-name=PS_s5_meas_strong_icx_16node
#SBATCH --output=stage5-scaling/results/measured/strong-icx-16node/slurm.out
#SBATCH --error=stage5-scaling/results/measured/strong-icx-16node/slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=16
#SBATCH --ntasks=1152
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:10:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage5-scaling/results/measured"
CASE_OUT="${OUT}/strong-icx-16node"
CFG="${REPO_ROOT}/shared/pisolver-triad-bench/configs/strong-icx-16node.cfg"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
measured_benchmark_prepare
TSV="${OUT}/stage5-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
OUTFILE="${CASE_OUT}/strong-icx-16node.out"
measured_srun_to_file 1152 "${CFG}" "${OUTFILE}"
read -r wall med nsteps < <(measured_parse_wall_seconds "${OUTFILE}")
pi=$(awk '/^piSize/{print $3}' "${CFG}")
arr=$(awk '/^arraySize/{print $3}' "${CFG}")
echo -e "strong-icx-16node\t${pi}\t${arr}\t${nsteps}\t1152\t${wall}\t${med}\tok" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "Stage 5 measured strong-icx-16node done: wall=${wall}s median_step=${med}s steps=${nsteps}"
