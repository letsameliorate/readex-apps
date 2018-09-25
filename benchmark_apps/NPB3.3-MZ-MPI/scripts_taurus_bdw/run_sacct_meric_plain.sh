#!/bin/sh

#SBATCH --time=04:00:00   # walltime
#SBATCH --nodes=1  # number of processor cores (i.e. tasks)
#SBATCH --ntasks-per-node=2
#SBATCH --cpus-per-task=14
#SBATCH --exclusive
#SBATCH --partition=broadwell
#SBATCH --reservation=p_readex_56
#SBATCH --mem-per-cpu=2000M   # memory per CPU core
#SBATCH -A p_readex
##SBATCH -J "bt_C_sacct_meric"   # job name
##SBATCH --output=bt_C_sacct_meric.out
##SBATCH --error=bt_C_sacct_meric.out
###############################################################################

#####
# set to number of times to repeat experiments
REPEAT_COUNT=5
#####

LOC=$(pwd)

cd ..

module purge
source ./readex_env/set_env_plain.source

cd $LOC
./compile_for_plain.sh
cd ..

i=1
rm -rf PLAIN_*
while [ $i -le $REPEAT_COUNT ]; do
  mkdir PLAIN_$i
  export MEASURE_RAPL_TARGET="PLAIN_$i"
  srun --cpu_bind=verbose,sockets measure-rapl ./bin/bt-mz.C.2_plain
  i=$(echo "$i + 1" | bc)
done

module purge
source ./readex_env/set_env_meric.source

cd $LOC
./compile_for_meric.sh
cd ..

# SET MERIC OUTPUT
#export MERIC_COUNTERS=perfevent
export MERIC_MODE=3
export MERIC_DEBUG=0
export MERIC_CONTINUAL=1
export MERIC_AGGREGATE=0
export MERIC_DETAILED=0
export MERIC_NUM_THREADS=0
export MERIC_FREQUENCY=0
export MERIC_UNCORE_FREQUENCY=0
export MERIC_REGION_OPTIONS=./energy.opts

i=1
rm -rf TUNED_*
while [ $i -le $REPEAT_COUNT ]; do
  mkdir TUNED_$i
  export MEASURE_RAPL_TARGET="TUNED_$i"
  srun --cpu_bind=verbose,sockets measure-rapl ./bin/bt-mz.C.2_meric
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

