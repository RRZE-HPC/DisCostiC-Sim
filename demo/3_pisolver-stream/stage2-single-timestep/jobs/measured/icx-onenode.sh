#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Optional Stage 2 test — measured Pi+STREAM, 1 node × 72 ranks.
#          Not part of run_job_chain.py. Submit by hand if you want this case.

#SBATCH --job-name=PS_s2_meas_onenode
#SBATCH --output=stage2-single-timestep/results/measured/icx-onenode-100/slurm.out
#SBATCH --error=stage2-single-timestep/results/measured/icx-onenode-100/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --time=00:10:00
#SBATCH --mem=60G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage2-single-timestep/results/measured"
CASE_OUT="${OUT}/icx-onenode-100"
CFG="${REPO_ROOT}/shared/pisolver-triad-bench/configs/icx-onenode-100.cfg"
mkdir -p "${CASE_OUT}" "${OUT}/tsv_fragments"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
export MEASURED_SRUN_EXTRA="-N 1 --ntasks-per-node=72"
# Bind all 72 ranks across both sockets (CPUs 0-71)
export MEASURED_CPU_BIND="map_cpu:$(seq -s, 0 71)"
measured_benchmark_prepare
TSV="${OUT}/stage2-measured-100.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
tmp="${CASE_OUT}/icx-onenode-100.out"
measured_srun_to_file 72 "${CFG}" "${tmp}"
read -r wall med nsteps < <(measured_parse_wall_seconds "${tmp}")
pi=$(awk '/^piSize/{print $3}' "${CFG}")
arr=$(awk '/^arraySize/{print $3}' "${CFG}")
echo -e "icx-onenode-100\t${pi}\t${arr}\t${nsteps}\t72\t${wall}\t${med}\tok" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "ICX onenode Stage 2 measured done wall=${wall}s median_step=${med}s nprocs=72"
