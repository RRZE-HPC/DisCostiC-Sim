#!/bin/bash
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Purpose: MPI environment setup shared by DisCostiC simulation and measured jobs.

if [ "${SLURM_JOB_NUM_NODES:-1}" -gt 1 ]; then
    export I_MPI_FABRICS="${I_MPI_FABRICS:-shm:ofi}"
    export I_MPI_OFI_PROVIDER="${I_MPI_OFI_PROVIDER:-verbs}"
    export FI_PROVIDER="${FI_PROVIDER:-verbs;ofi_rxm}"
fi

# Build srun options for dense socket jobs (e.g. SPR 104 ranks/node).
discostic_srun_opts() {
    DISCOSTIC_SRUN_OPTS=()
    if [ "${SLURM_JOB_NUM_NODES:-1}" -le 1 ]; then
        return 0
    fi
    local tpn
    tpn=$(awk -F= '/^task_per_node/ {gsub(/#.*/,"",$2); gsub(/ /,"",$2); print $2; exit}' config.cfg 2>/dev/null || true)
    if [ -n "${tpn}" ] && [ "${tpn}" -gt 1 ]; then
        DISCOSTIC_SRUN_OPTS=(--ntasks-per-node="${tpn}" --cpu-bind=cores)
    elif [ -n "${tpn}" ] && [ "${tpn}" -eq 1 ]; then
        # Match measured tpn=1 layout: one task per node across the allocation.
        DISCOSTIC_SRUN_OPTS=(--ntasks-per-node=1)
    fi
}

discostic_srun() {
    discostic_srun_opts
    if [ -n "${DISCOSTIC_USE_MPIRUN:-}" ]; then
        mpirun -n "${TC_PROCNUMBERS}" ./discostic
    else
        # shellcheck disable=SC2086
        srun -n "${TC_PROCNUMBERS}" \
            ${DISCOSTIC_CPU_FREQ:+--cpu-freq="${DISCOSTIC_CPU_FREQ}"} \
            ${DISCOSTIC_SRUN_OPTS[@]+"${DISCOSTIC_SRUN_OPTS[@]}"} ./discostic
    fi
}
