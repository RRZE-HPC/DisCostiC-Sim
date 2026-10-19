#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: Stage 3 measured pisolver-triad-bench ICX tpn=1 (100/1000/10000).

#SBATCH --job-name=PS_s3_meas_icx-tpn1
#SBATCH --output=stage3-multi-timestep/results/measured/slurm-icx-tpn1.out
#SBATCH --error=stage3-multi-timestep/results/measured/slurm-icx-tpn1.err
#SBATCH --partition=multinode
#SBATCH --nodes=36
#SBATCH --ntasks=36
#SBATCH --ntasks-per-node=1
#SBATCH --time=01:19:00
#SBATCH --mem=10G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage3-multi-timestep/results/measured"
BENCH_CFG_DIR="${REPO_ROOT}/shared/pisolver-triad-bench/configs"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
export MEASURED_CPU_BIND="map_cpu:0"
export MEASURED_SRUN_EXTRA="-N 36 --ntasks-per-node=1"
measured_benchmark_prepare
TSV="${OUT}/stage3-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
: > "${FRAG}"
work_cfg="$(mktemp)"
for iters in 100 1000 10000; do
  awk -v n=$((iters - 1)) '
    /^numTimeStep/ { print "numTimeStep = " n; next }
    { print }
  ' "${BENCH_CFG_DIR}/icx-tpn1-100.cfg" > "${work_cfg}"
  out="${OUT}/icx-tpn1-${iters}.out"
  measured_srun_to_file 36 "${work_cfg}" "${out}"
  read -r wall med nsteps < <(measured_parse_wall_seconds "${out}")
  pi=$(awk '/^piSize/{print $3}' "${work_cfg}")
  arr=$(awk '/^arraySize/{print $3}' "${work_cfg}")
  echo -e "icx-tpn1-${iters}\t${pi}\t${arr}\t${nsteps}\t36\t${wall}\t${med}\tok" >> "${FRAG}"
done
rm -f "${work_cfg}"
measured_tsv_append "${TSV}" "${FRAG}"
