#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 3 O1 case-study measured STREAM Triad on ICX (tpn=1) for 100/1000/10000 timesteps.

#SBATCH --job-name=TC_sO1_3_icx_tpn1
#SBATCH --output=stage3-multi-timestep/results/measured-o1/icx-tpn1-slurm.out
#SBATCH --error=stage3-multi-timestep/results/measured-o1/icx-tpn1-slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=36
#SBATCH --ntasks=36
#SBATCH --time=00:17:00
#SBATCH --mem=10G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}"
OUT="${REPO_ROOT}/stage3-multi-timestep/results/measured-o1"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
# One rank per node on core 0 (socket 0), matching the stage 2 tpn=1 placement.
export MEASURED_CPU_BIND="map_cpu:0"
export MEASURED_SRUN_EXTRA="-N 36 --ntasks-per-node=1"
export MEASURED_BENCH_OPT=O1
measured_benchmark_prepare
TSV="${OUT}/stage3-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
: > "${FRAG}"
for iters in 100 1000 10000; do
  out="${OUT}/icx-tpn1-${iters}.out"
  measured_srun_to_file 36 1000000000 "${iters}" "${out}"
  line=$(grep '^wall_seconds=' "${out}" | tail -1)
  wall=$(echo "${line}" | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
  bw=$(echo "${line}" | sed -n 's/.*bw_gb_s=\([^ ]*\).*/\1/p')
  gflops=$(echo "${line}" | sed -n 's/.*gflops=\([^ ]*\).*/\1/p')
  chk=$(echo "${line}" | sed -n 's/.*check=\([^ ]*\).*/\1/p')
  echo -e "icx-tpn1-${iters}\t1000000000\t${iters}\t36\t${wall}\t${bw}\t${gflops}\t${chk}" >> "${FRAG}"
done
measured_tsv_append "${TSV}" "${FRAG}"
