#!/bin/sh

source readex_env/set_env_rdd.source

pathToFilter=$(pwd)

export SCOREP_PREP="scorep --online-access --user --nocompiler --mpp=mpi --thread=none"
export DSWITCH="-DUSE_SCOREP_MANUAL"

make clean
rm -rf miniMD_openmpi miniMD_openmpi_rdd
make openmpi
mv miniMD_openmpi miniMD_openmpi_rdd
