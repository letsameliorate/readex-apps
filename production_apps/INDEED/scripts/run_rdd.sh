#!/bin/bash

# usage: ./run_rdd.sh [exampleID] 
#        where exampleID may be "B1", "SBC" or "H1N"
#        default value is B1


if [ $# -eq 0 ]; then
    ARG="B1"
else
    ARG=`echo $1 | tr [a-z] [A-Z]`;
fi

if [ $ARG == "SBC" ]; then
    JOBNAME="SBC-OMP-I16";
    JOBTIME="01:00:00";
    JOBCPUS=24;
    JOBMEM=28000;
    export SCOREP_TOTAL_MEMORY=2000M
elif [ $ARG == "B1" ]; then
    JOBNAME="B1-OMP-I16";
    JOBTIME="00:45:00";
    JOBCPUS=24;
    JOBMEM=8000;
    export SCOREP_TOTAL_MEMORY=200M
elif [ $ARG == "H1N" ]; then
    JOBNAME="H1N-OMP-I16";
    JOBTIME="00:10:00"
    JOBCPUS=24;
    JOBMEM=6000;
    export SCOREP_TOTAL_MEMORY=200M
else
    echo $0: Illegal exampleID $1
    echo Legal values are "B1", "SBC" and "H1N"
    exit 1
fi

export RUNTYPE="rdd"

export RDD_GRANULARITY_THRESHOLD="0.1"
export RDD_PHASE_REGION_NAME="mainloop"
export RDD_COMPUTE_INTENSITY_VARIATION_THRESHOLD="5"
export RDD_EXEC_TIME_VARIATION_THRESHOLD="5"
export RDD_REGION_EXEC_TIME_VS_PHASE_EXEC_TIME="3"
export RDD_CONFIG_FILE="readex_config_Indeed.xml"
export RDD_RADAR_REPORT_FILE="radar.tex"

CURRDIR=`pwd`
cd `dirname $0`

if [ `echo ${MYMAILADDRESS:=none}` != "none" ]; then
    ADDARG="--mail-user=$MYMAILADDRESS"
else
    ADDARG=""
fi

MY_SLURM_ID=`sbatch -J $JOBNAME --time=$JOBTIME $ADDARG --cpus-per-task=$JOBCPUS --mem-per-cpu=$JOBMEM submitIndeed_"$RUNTYPE".sbatch | cut -d" " -f4`
echo '##' Command: sbatch -J $JOBNAME --time=$JOBTIME $ADDARG --cpus-per-task=$JOBCPUS --mem-per-cpu=$JOBMEM submitIndeed_"$RUNTYPE".sbatch > submitIndeed_"$RUNTYPE"_$MY_SLURM_ID.sbatch
echo '##' was executed with following file: >> submitIndeed_"$RUNTYPE"_$MY_SLURM_ID.sbatch
echo >> submitIndeed_"$RUNTYPE"_$MY_SLURM_ID.sbatch
cat submitIndeed_"$RUNTYPE".sbatch >> submitIndeed_"$RUNTYPE"_$MY_SLURM_ID.sbatch

echo Job $MY_SLURM_ID submitted.

cd $CURRDIR

