#!/bin/bash -l
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: SLURM job: Stage 4 measured STREAM idlewave with Intel Trace Collector (.stf) on SPR.
#          Separate from the scoring measured job so ITAC overhead does not change wall_seconds.

#SBATCH --job-name=TC_s4_mtr_spr
#SBATCH --output=stage4-idlewave/results/measured-trace/spr/slurm.out
#SBATCH --error=stage4-idlewave/results/measured-trace/spr/slurm.err
#SBATCH --partition=spr1tb
#SBATCH --nodes=1
#SBATCH --ntasks=104
#SBATCH --ntasks-per-node=104
#SBATCH --exclusive
#SBATCH --mem=130G
#SBATCH --time=00:20:00
#SBATCH --export=NONE
#SBATCH --cpu-freq=2000000-2000000:performance
set -euo pipefail
unset SLURM_EXPORT_ENV
REPO_ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)}"
OUT="${REPO_ROOT}/stage4-idlewave/results/measured-trace"
CASE_OUT="${OUT}/spr"
mkdir -p "${CASE_OUT}"
module load intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null || module load intel/2025.2.0 intelmpi/2021.16.0
module load intel-itac/2022.4.0 2>/dev/null || module load intel-itac 2>/dev/null || module load itac 2>/dev/null || true
source "${REPO_ROOT}/shared/scripts/measured_common.sh"
export MEASURED_CPU_FREQ=2000000-2000000:performance
measured_benchmark_prepare
OUTFILE="${CASE_OUT}/spr-trace.out"
measured_srun_traced 104 2000000000 45 "${OUTFILE}" "stream-triad-idlewave-spr-measured"
echo "Stage 4 measured-trace SPR done: ${CASE_OUT}/stream-triad-idlewave-spr-measured.single.stf"
