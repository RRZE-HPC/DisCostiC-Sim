#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 2 O1 case-study measured STREAM Triad on SPR socket.

#SBATCH --job-name=TC_sO1_2_spr
#SBATCH --output=stage2-single-timestep/results/measured-o1/spr-slurm.out
#SBATCH --error=stage2-single-timestep/results/measured-o1/spr-slurm.err
#SBATCH --partition=spr1tb
#SBATCH --nodes=1
#SBATCH --ntasks=52
#SBATCH --exclusive
#SBATCH --time=00:10:00
#SBATCH --mem=60G
#SBATCH --export=NONE
#SBATCH --cpu-freq=2000000-2000000:performance
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
# Pin to socket 0 = CPUs 0-51 (Fritz SPR: 2 sockets × 52, 8 NUMA / SNC-4).
# Do not use numactl --cpunodebind=0 alone — that is only 1/4 of a socket (13 cores).
# Unpinned srun spans both sockets; DisCostiC and the ECM model one socket.
# Slurm map_cpu requires a comma list (ranges like 0-51 are rejected).
srun --cpu-freq=2000000-2000000:performance \
    --cpu-bind=map_cpu:$(seq -s, 0 51) -n 52 \
    ./mpi_stream_bench_comm_O1 1073741824 100 | tee "${OUT}/spr-socket-100.out"
line=$(grep '^wall_seconds=' "${OUT}/spr-socket-100.out" | tail -1)
wall=$(echo "${line}" | sed -n 's/.*wall_seconds=\([^ ]*\).*/\1/p')
bw=$(echo "${line}" | sed -n 's/.*bw_gb_s=\([^ ]*\).*/\1/p')
gflops=$(echo "${line}" | sed -n 's/.*gflops=\([^ ]*\).*/\1/p')
chk=$(echo "${line}" | sed -n 's/.*check=\([^ ]*\).*/\1/p')
echo -e "spr-socket-100\t1073741824\t100\t52\t${wall}\t${bw}\t${gflops}\t${chk}" > "${FRAG}"
measured_tsv_append "${TSV}" "${FRAG}"
