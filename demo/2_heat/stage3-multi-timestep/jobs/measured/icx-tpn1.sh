#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
#
# Purpose: SLURM job: Stage 3 measured HEAT Jacobi 2D on ICX tpn=1.

#SBATCH --job-name=HEAT_s3_icx_tpn1
#SBATCH --output=stage3-multi-timestep/results/measured/icx-tpn1-slurm.out
#SBATCH --error=stage3-multi-timestep/results/measured/icx-tpn1-slurm.err
#SBATCH --partition=multinode
#SBATCH --nodes=36
#SBATCH --ntasks=36
#SBATCH --time=00:18:00
#SBATCH --mem=10G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage3-multi-timestep/results/measured"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
export MEASURED_CPU_BIND="map_cpu:0"
export MEASURED_SRUN_EXTRA="-N 36 --ntasks-per-node=1"
measured_benchmark_prepare
TSV="${OUT}/stage3-measured.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
: > "${FRAG}"
for iters in 100 1000 10000; do
  out="${OUT}/icx-tpn1-${iters}.out"
  measured_srun_to_file 36 92160 16384 "${iters}" "${out}"
  wall=$(grep '^wall_seconds=' "${out}" | tail -1 | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
  echo -e "icx-tpn1-${iters}\t92160\t16384\t${iters}\t36\t${wall}" >> "${FRAG}"
done
measured_tsv_append "${TSV}" "${FRAG}"
