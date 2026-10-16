#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 4 measured Pi+STREAM idlewave with Intel Trace Collector (.stf).
#          Separate from the scoring measured job so ITAC overhead does not change wall_seconds.

#SBATCH --job-name=PS_s4_mtr_icx
#SBATCH --output=stage4-idlewave/results/measured-trace/icx/slurm.out
#SBATCH --error=stage4-idlewave/results/measured-trace/icx/slurm.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --ntasks-per-node=72
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:10:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage4-idlewave/results/measured-trace"
CASE_OUT="${OUT}/icx"
CFG="${REPO_ROOT}/shared/pisolver-triad-bench/configs/icx-idlewave.cfg"
mkdir -p "${CASE_OUT}"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
module load intel-itac/2022.4.0 2>/dev/null || module load intel-itac 2>/dev/null || module load itac 2>/dev/null || true
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2400000-2400000:performance
measured_benchmark_prepare
OUTFILE="${CASE_OUT}/icx-trace.out"
measured_srun_traced 72 "${CFG}" "${OUTFILE}" "pitriadlbl-idlewave-measured"
echo "Stage 4 measured-trace ICX done: ${CASE_OUT}/pitriadlbl-idlewave-measured.single.stf"
