#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 3 measured STREAM Triad on SPR socket for 100/1000/10000 timesteps.

#SBATCH --job-name=TC_s3_spr
#SBATCH --output=stage3-multi-timestep/results/measured/spr-slurm.out
#SBATCH --error=stage3-multi-timestep/results/measured/spr-slurm.err
#SBATCH --partition=spr1tb
#SBATCH --nodes=1
#SBATCH --ntasks=52
#SBATCH --exclusive
#SBATCH --time=00:45:00
#SBATCH --mem=60G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2000000-2000000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}"
OUT="${REPO_ROOT}/stage3-multi-timestep/results/measured"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2000000-2000000:performance
# Full SPR socket 0 (CPUs 0-51); SNC-4 has 8 NUMA nodes — do not use numactl node 0 alone.
export MEASURED_CPU_BIND="map_cpu:$(seq -s, 0 51)"
measured_benchmark_prepare
TSV="${OUT}/stage3-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
: > "${FRAG}"
for iters in 100 1000 10000; do
  out="${OUT}/spr-socket-${iters}.out"
  measured_srun_to_file 52 1073741824 "${iters}" "${out}"
  line=$(grep '^wall_seconds=' "${out}" | tail -1)
  wall=$(echo "${line}" | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
  bw=$(echo "${line}" | sed -n 's/.*bw_gb_s=\([^ ]*\).*/\1/p')
  gflops=$(echo "${line}" | sed -n 's/.*gflops=\([^ ]*\).*/\1/p')
  chk=$(echo "${line}" | sed -n 's/.*check=\([^ ]*\).*/\1/p')
  echo -e "spr-socket-${iters}\t1073741824\t${iters}\t52\t${wall}\t${bw}\t${gflops}\t${chk}" >> "${FRAG}"
done
measured_tsv_append "${TSV}" "${FRAG}"
