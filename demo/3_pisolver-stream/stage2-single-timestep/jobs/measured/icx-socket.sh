#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Stage 2 measured pisolver-triad-bench on ICX socket.

#SBATCH --job-name=PS_s2_meas_icx
#SBATCH --output=stage2-single-timestep/results/measured/slurm-icx.out
#SBATCH --error=stage2-single-timestep/results/measured/slurm-icx.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=36
#SBATCH --exclusive
#SBATCH --time=00:10:00
#SBATCH --mem=60G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage2-single-timestep/results/measured"
CFG="${REPO_ROOT}/shared/pisolver-triad-bench/configs/icx-socket-100.cfg"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
export MEASURED_CPU_BIND="map_cpu:$(seq -s, 0 35)"
measured_benchmark_prepare
TSV="${OUT}/stage2-measured-100.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
tmp="${OUT}/icx-socket-100.out"
measured_srun_to_file 36 "${CFG}" "${tmp}"
read -r wall med nsteps < <(measured_parse_wall_seconds "${tmp}")
pi=$(awk '/^piSize/{print $3}' "${CFG}")
arr=$(awk '/^arraySize/{print $3}' "${CFG}")
echo -e "icx-socket-100\t${pi}\t${arr}\t${nsteps}\t36\t${wall}\t${med}\tok" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "ICX socket Stage 2 measured done wall=${wall}s median_step=${med}s"
