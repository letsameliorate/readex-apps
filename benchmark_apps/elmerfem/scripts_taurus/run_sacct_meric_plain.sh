#!/bin/sh

#SBATCH --time=2:00:00
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=24
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH --partition=haswell
#SBATCH --comment="no_monitoring"
#SBATCH --mem-per-cpu=2500M
#SBATCH -J "ELMER-compare"
#SBATCH -A p_readex
#SBATCH --reservation=READEX

module purge
SCRIPT_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
. ${SCRIPT_DIR}/readex_env/set_env_meric.source
. ${SCRIPT_DIR}/../paths.source

#cd ${ELMER_ROOT}

NPROCS=96

REPEAT_COUNT=5

export PROBLEM_PATH="${ELMER_ROOT}/build/fem/tests/WinkelPoissonPartitionUniform/"

cd  ${PROBLEM_PATH}

cp meric.opts .
cp readex_config_extended.xml .
cp ${ELMER_ROOT}/case_cg.sif case.sif
cp ${ELMER_ROOT}/scorep.filt .

echo "Generating Grid for Elmer..."
if [ ! -d ${PROBLEM_PATH}/winkel/partitioning.96 ]; then
    echo "Problem winkel generated."
    ${ELMER_ROOT}/gen_problem.sh
else
    echo "Problem winkel already exists."
fi

export MERIC_MODE=3
export MERIC_FREQUENCY=0
export MERIC_UNCORE_FREQUENCY=0
export MERIC_NUM_THREADS=0

i=1
rm -rf PLAIN_*
while [ $i -le $REPEAT_COUNT ]; do
  mkdir PLAIN_$i
  export MEASURE_RAPL_TARGET="PLAIN_$i"
  srun -n $NPROCS measure-rapl ElmerSolver_mpi
  i=$(echo "$i + 1" | bc)
done


export MERIC_REGION_OPTIONS=meric.opts

i=1
rm -rf TUNED_*
while [ $i -le $REPEAT_COUNT ]; do
  mkdir TUNED_$i
  export MEASURE_RAPL_TARGET="TUNED_$i"
  srun -n $NPROCS measure-rapl ElmerSolver_mpi
  i=$(echo "$i + 1" | bc)
done

i=1
total_time_plain=0
total_energy_plain=0
total_cpu_energy_plain=0
while [ $i -lt $REPEAT_COUNT ]; do
  times_energys=$(sacct -j $SLURM_JOBID.$i --format="JobID,CPUTimeRAW,ConsumedEnergyRaw")
  i=$(echo "$i + 1" | bc)
  times_energys_array=(${times_energys[@]})
  time_step=${times_energys_array[7]}
  energy_step=${times_energys_array[8]}
  total_time_plain=$(echo "${total_time_plain} + ${time_step}" | bc)
  total_energy_plain=$(echo "${total_energy_plain} + ${energy_step}" | bc)
  for file in PLAIN_$i/*
  do
    values=$( tail -1 $file | awk -F'[ ,]' '{print int($1)" "int($2)}' )
    values=(${values[@]})
    total_cpu_energy_plain=$[ total_cpu_energy_plain + ${values[0]} + ${values[1]} ]
  done
done

i=1
total_time_rrl=0
total_energy_rrl=0
total_cpu_energy_rrl=0
while [ $i -lt $REPEAT_COUNT ]; do
  times_energys=$(sacct -j $SLURM_JOBID.$((i+REPEAT_COUNT)) --format="JobID,CPUTimeRAW,ConsumedEnergyRaw")
  i=$(echo "$i + 1" | bc)
  times_energys_array=(${times_energys[@]})
  time_step=${times_energys_array[7]}
  energy_step=${times_energys_array[8]}
  total_time_rrl=$(echo "${total_time_rrl} + ${time_step}" | bc)
  total_energy_rrl=$(echo "${total_energy_rrl} + ${energy_step}" | bc)
  for file in TUNED_$i/*
  do
    values=$( tail -1 $file | awk -F'[ ,]' '{print int($1)" "int($2)}' )
    values=(${values[@]})
    total_cpu_energy_rrl=$[ total_cpu_energy_rrl + ${values[0]} + ${values[1]} ]
  done
done

echo ""
echo "Total Plain Time = $total_time_plain, Total Plain Energy = $total_energy_plain"
echo "Total MERIC Time = $total_time_rrl, Total MERIC Energy = $total_energy_rrl"

avg_time_plain=$(echo "$total_time_plain / $((REPEAT_COUNT-1))" | bc)
avg_energy_plain=$(echo "$total_energy_plain / $((REPEAT_COUNT-1))" | bc)
avg_time_rrl=$(echo "$total_time_rrl / $((REPEAT_COUNT-1))" | bc)
avg_energy_rrl=$(echo "$total_energy_rrl / $((REPEAT_COUNT-1))" | bc)
avg_cpu_energy_plain=$(echo "$total_cpu_energy_plain / $((REPEAT_COUNT-1))" | bc)
avg_cpu_energy_rrl=$(echo "$total_cpu_energy_rrl / $((REPEAT_COUNT-1))" | bc)

echo ""
echo "Average Plain Time = $avg_time_plain, Average MERIC Time = $avg_time_rrl"
echo "Average Plain Energy = $avg_energy_plain, Average MERIC Energy = $avg_energy_rrl"
echo "Average Plain CPU Energy = $avg_cpu_energy_plain, Average MERIC CPU Energy = $avg_cpu_energy_rrl"

rm -rf PLAIN_*
rm -rf TUNED_*

