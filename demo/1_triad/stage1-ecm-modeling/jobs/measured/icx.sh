#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 1 measured STREAM Triad on ICX single socket.

#SBATCH --job-name=TC_s1_measured
#SBATCH --output=stage1-ecm-modeling/results/measured/slurm.out
#SBATCH --error=stage1-ecm-modeling/results/measured/slurm.err
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
ROOT="${DISCOSTIC_ROOT:-${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}}"
OUT="${REPO_ROOT}/stage1-ecm-modeling/results/measured"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
measured_benchmark_prepare
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
: > "${FRAG}"
tmp="/tmp/tc_s1_icx-socket-100.out"
srun --cpu-freq=2400000-2400000:performance \
    --cpu-bind=map_cpu:$(seq -s, 0 35) -n 36 \
    ./mpi_stream_bench_comm 1073741824 100 > "${tmp}" 2>&1
cp "${tmp}" "${OUT}/icx-socket-100.out"
line=$(grep '^wall_seconds=' "${tmp}" | tail -1)
wall=$(echo "${line}" | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
bw=$(echo "${line}" | sed -n 's/.*bw_gb_s=\([^ ]*\).*/\1/p')
gflops=$(echo "${line}" | sed -n 's/.*gflops=\([^ ]*\).*/\1/p')
chk=$(echo "${line}" | sed -n 's/.*check=\([^ ]*\).*/\1/p')
echo -e "icx-socket-100\t1073741824\t100\t36\t${wall}\t${bw}\t${gflops}\t${chk}" >> "${FRAG}"
measured_tsv_append "${OUT}/stage1-measured.tsv" "${FRAG}"
echo "ICX socket done. SPR requires spr1tb partition — see jobs/measured/spr.sh"
