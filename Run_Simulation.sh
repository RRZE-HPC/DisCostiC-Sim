#!/bin/bash -l
#SBATCH --job-name=DisCostiC_Simulation
#SBATCH --output=DisCostiC_Simulation.%j.out
#SBATCH --error=DisCostiC_Simulation.%j.err
#SBATCH --partition=singlenode
#SBATCH --nodes=1
#SBATCH --ntasks=72
#SBATCH --mem=60G
#SBATCH --time=00:20:00
#SBATCH --export=NONE
# Purpose: Fritz test job. Reads the repository config.cfg as the user left it.
set -euo pipefail
unset SLURM_EXPORT_ENV
cd "${SLURM_SUBMIT_DIR:-$(pwd)}"

module load git cmake likwid intel/2023.2.1 intelmpi/2021.16.0 2>/dev/null \
    || module load git cmake likwid intel/2025.2.0 intelmpi/2021.16.0

VENV="${PWD}/.venv-discostic"
if [ ! -d "${VENV}" ]; then
    /usr/bin/python3 -m venv "${VENV}"
fi
# shellcheck disable=SC1091
source "${VENV}/bin/activate"
python -c "import kerncraft" 2>/dev/null || python -m pip install --quiet kerncraft numpy osaca

cfg_val() {
    awk -F= -v key="$1" '
        $0 ~ "^" key "[ \t]*=" {
            sub(/^[^#=]*=[ \t]*/, "")
            sub(/#.*/, "")
            gsub(/^[ \t]+|[ \t]+$/, "")
            print
            exit
        }
    ' config.cfg
}

FILENAME=$(cfg_val benchmark_kernel)
PROCNUMBERS=$(cfg_val number_of_processes)
HETEROGENEOUS=$(cfg_val heteregeneous)
PART2=$(cfg_val part2)
if [ -z "${FILENAME}" ] || [ -z "${PROCNUMBERS}" ]; then
    echo "ERROR: config.cfg must set benchmark_kernel and number_of_processes" >&2
    exit 1
fi
TOTALPROCS=$(( PROCNUMBERS + 1 ))
if [ "${HETEROGENEOUS:-0}" -eq 1 ]; then
    SECONDARY=$(cfg_val secondary_number_of_processes)
    TOTALPROCS=$(( TOTALPROCS + ${SECONDARY:-0} ))
fi
if [ "${TOTALPROCS}" -gt "${SLURM_NTASKS:-1}" ]; then
    echo "ERROR: config.cfg needs ${TOTALPROCS} ranks but this allocation has ${SLURM_NTASKS:-1}." >&2
    echo "Raise #SBATCH --ntasks, or lower number_of_processes." >&2
    exit 1
fi
if [ ! -f "./test/${FILENAME}.hpp" ]; then
    echo "ERROR: test/${FILENAME}.hpp is not present" >&2
    exit 1
fi

cp "./test/${FILENAME}.hpp" ./test/P2P.hpp
restore_header() {
    git checkout -- test/P2P.hpp 2>/dev/null || true
}
trap restore_header EXIT

if [ ! -d build ]; then
    cmake -S . -B build
fi
make -C build discostic
if [ ! -x ./discostic ] && [ -x ./build/discostic ]; then
    cp ./build/discostic ./discostic
fi

if grep -qE 'FILE:|SRC:' ./test/P2P.hpp; then
    KERNEL=$(grep -oE '(FILE|SRC):[^" ]+\.c' ./test/P2P.hpp | head -1 | cut -d: -f2 || true)
    KERNEL_PATH="nodelevel/kernels/${KERNEL}"
    if [ -n "${KERNEL}" ] && [ -f "${KERNEL_PATH}" ]; then
        ARCH=$(cfg_val micro_architecture)
        PMODEL=$(cfg_val pmodel)
        CPRED=$(cfg_val cache_predictor)
        FLAGS=$(cfg_val compiler_flags)
        TPN=$(cfg_val task_per_node)
        N_CFG=$(cfg_val N)
        CORES=$(grep -m1 "^cores per socket" "nodelevel/machine-files/${ARCH}.yml" | awk '{print $NF}')
        [ -z "${CORES}" ] && CORES="${TPN}"
        if [ "${TPN}" -lt "${CORES}" ]; then
            ECM_CORE="${TPN}"
        else
            ECM_CORE="${CORES}"
        fi
        BASE_ECM="nodelevel/configs/${FILENAME}.cfg"
        DEFINE_ARGS=()
        if [ -n "${N_CFG}" ]; then
            NLOCAL=$(( N_CFG / PROCNUMBERS ))
            DEFINE_N=$(( NLOCAL * ECM_CORE ))
            DEFINE_ARGS+=(-D N "${DEFINE_N}")
        fi
        if [ "${KERNEL}" = "stream-triad.c" ]; then
            DEFINE_ARGS+=(-D s 55)
        fi
        echo "[ECM pre-gen] ${KERNEL_PATH} -> ${BASE_ECM}"
        python3 ./kerncraftintegration/diskern.py \
            -m"./nodelevel/machine-files/${ARCH}.yml" \
            -c"${ECM_CORE}" \
            -o"${BASE_ECM}" \
            -extras"${ECM_CORE}+0" \
            -systemNo0 \
            -p"${PMODEL}" \
            -P"${CPRED}" \
            --compiler-flags="${FLAGS}" \
            "${DEFINE_ARGS[@]}" \
            "${KERNEL_PATH}"
        if [ ! -f "${BASE_ECM}" ]; then
            echo "ERROR: diskern.py did not write ${BASE_ECM}" >&2
            exit 1
        fi
        for num in $(seq 0 "${TOTALPROCS}"); do
            cp "${BASE_ECM}" "nodelevel/configs/${FILENAME}${num}.cfg"
        done
    fi
fi

run_pass() {
    if [ -f "nodelevel/configs/${FILENAME}.cfg" ]; then
        for num in $(seq 0 "${TOTALPROCS}"); do
            cp "nodelevel/configs/${FILENAME}.cfg" "nodelevel/configs/${FILENAME}${num}.cfg"
        done
    fi
    srun -n "${TOTALPROCS}" ./discostic
}

run_pass
if [ -n "${PART2}" ] && [ "${PART2}" != "0" ]; then
    run_pass
fi
