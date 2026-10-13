#!/bin/bash -l
#SBATCH --job-name=DisCostiC_Simulation
#SBATCH --output=DisCostiC_Simulation.o%j
#SBATCH --partition=multinode
#SBATCH --nodes=2
#SBATCH --time=00:10:00
#SBATCH --constraint=hwperf
#SBATCH --export=NONE
#SBATCH --cpu-freq=2400000-2400000:performance

unset SLURM_EXPORT_ENV 

echo "Starting: $(date)"

export 
cd $SLURM_SUBMIT_DIR

module load python git cmake likwid itac
module use ~unrz139/.modules/modulefiles
module load vampir
module list
conda activate $HOME/.conda/evs/XYZ

PROCNUMBERS=$(cat config.cfg | grep "number_of_processes" | head -1 | sed 's/[^0-9]*//g')
PROCNUMBERS=$(( PROCNUMBERS + 1 ))

export VT_FLUSH_PREFIX=/tmp
export VT_LOGFILE_FORMAT=SINGLESTF


FILENAME=$(awk -F= '/benchmark_kernel/ {gsub(/#.*/,"",$2); gsub(/^[ \t]+|[ \t]+$/,"",$2); print $2}' config.cfg)

FILEMODE=$FILENAME".hpp"
FILEMODE=$(echo $FILEMODE | tr -d " ")

if test -f "./test/$FILEMODE"; then
    cp ./test/$FILEMODE ./test/P2P.hpp
else
    echo ""
    echo "TestCase not present !!"
    echo "Exiting from Simulation !!"

    exit 1
fi

make

HETEROGENEOUS=$(cat config.cfg | grep "heteregeneous" | cut -d"#" -f1 $1 | sed 's/[^0-9]*//g')

if [ "$HETEROGENEOUS" -eq "0" ]; then
    TOTALPROCS=$(( $PROCNUMBERS ))
else
    SECONDARYSYSTEMNUMBERPROCS=$(cat config.cfg | grep "secondary_number_of_processes" | sed 's/[^0-9]*//g')
    TOTALPROCS=$(( $PROCNUMBERS+$SECONDARYSYSTEMNUMBERPROCS ))
fi

srun -n $TOTALPROCS ./discostic

PART1=$(awk -F= '/part1/ {gsub(/#.*/,"",$2); gsub(/^[ \t]+|[ \t]+$/,"",$2); print $2}' config.cfg)
PART2=$(awk -F= '/part2/ {gsub(/#.*/,"",$2); gsub(/^[ \t]+|[ \t]+$/,"",$2); print $2}' config.cfg)

if [ "$PART2" = "0" ]; then
    for i in $(seq 0 $TOTALPROCS); do rm "./nodelevel/configs/${FILENAME}${i}.cfg" 2>/dev/null; done
else
    srun -n $TOTALPROCS ./discostic
    for i in $(seq 0 $TOTALPROCS); do rm "./nodelevel/configs/${PART1}${i}.cfg" 2>/dev/null; done
    for i in $(seq 0 $TOTALPROCS); do rm "./nodelevel/configs/${PART2}${i}.cfg" 2>/dev/null; done
fi
