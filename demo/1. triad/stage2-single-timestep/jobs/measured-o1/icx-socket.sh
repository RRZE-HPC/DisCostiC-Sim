#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 2 O1 case-study measured STREAM Triad on ICX socket.

#SBATCH --job-name=TC_sO1_2_measured
#SBATCH --output=stage2-single-timestep/results/measured-o1/slurm.out
#SBATCH --error=stage2-single-timestep/results/measured-o1/slurm.err
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
ROOT="${DISCOSTIC_EXTENSION_ROOT:-$(cd "${REPO_ROOT}/../.." && pwd)}"
OUT="${REPO_ROOT}/stage2-single-timestep/results/measured-o1"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_BENCH_OPT=O1
measured_benchmark_prepare
TSV="${OUT}/stage2-measured-100.tsv"
FRAG="${OUT}/tsv_fragments/job_${SLURM_JOB_ID}.tsv"
tmp="/tmp/tc_s2_icx-socket-100.out"
# Pin to socket 0 = CPUs 0-35 (Fritz ICX: 2 sockets × 36, 4 NUMA / SNC-2).
# Do not use numactl --cpunodebind=0 alone — that is only half a socket (18 cores).
# Unpinned srun spans both sockets (~300 GB/s); DisCosTiC/ECM model one socket.
# Slurm map_cpu requires a comma list (ranges like 0-35 are rejected).
srun --cpu-freq=2400000-2400000:performance \
    --cpu-bind=map_cpu:$(seq -s, 0 35) -n 36 \
    ./mpi_stream_bench_comm_O1 1073741824 100 > "${tmp}" 2>&1
cp "${tmp}" "${OUT}/icx-socket-100.out"
line=$(grep '^wall_seconds=' "${tmp}" | tail -1)
wall=$(echo "${line}" | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
bw=$(echo "${line}" | sed -n 's/.*bw_gb_s=\([^ ]*\).*/\1/p')
gflops=$(echo "${line}" | sed -n 's/.*gflops=\([^ ]*\).*/\1/p')
chk=$(echo "${line}" | sed -n 's/.*check=\([^ ]*\).*/\1/p')
echo -e "icx-socket-100\t1073741824\t100\t36\t${wall}\t${bw}\t${gflops}\t${chk}" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
echo "ICX socket done"
