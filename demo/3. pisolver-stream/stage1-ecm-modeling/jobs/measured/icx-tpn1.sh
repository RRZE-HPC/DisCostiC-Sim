#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Stage 1 measured pisolver-triad-bench ICX tpn=1.

#SBATCH --job-name=PS_s1_tpn1
#SBATCH --output=stage1-ecm-modeling/results/measured/slurm-icx-tpn1.out
#SBATCH --error=stage1-ecm-modeling/results/measured/slurm-icx-tpn1.err
#SBATCH --partition=multinode
#SBATCH --nodes=36
#SBATCH --ntasks=36
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:10:00
#SBATCH --mem=10G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage1-ecm-modeling/results/measured"
CFG="${REPO_ROOT}/shared/pisolver-triad-bench/configs/icx-tpn1-100.cfg"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
export MEASURED_CPU_BIND="map_cpu:0"
export MEASURED_SRUN_EXTRA="-N 36 --ntasks-per-node=1"
measured_benchmark_prepare
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
tmp="${OUT}/icx-tpn1-100.out"
measured_srun_to_file 36 "${CFG}" "${tmp}"
read -r wall med nsteps < <(measured_parse_wall_seconds "${tmp}")
pi=$(awk '/^piSize/{print $3}' "${CFG}")
arr=$(awk '/^arraySize/{print $3}' "${CFG}")
echo -e "icx-tpn1-100\t${pi}\t${arr}\t${nsteps}\t36\t${wall}\t${med}\tok" > "${FRAG}"
measured_tsv_append "${OUT}/stage1-measured.tsv" "${FRAG}"
echo "ICX tpn1 Stage 1 done wall=${wall}s"
