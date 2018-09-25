#!/bin/sh

source ./readex_env/set_env_ptf_hdeem.source

pathToFilter=$(pwd)

export SCOREP_PREP="scorep --online-access --user --mpp=mpi --thread=none"
export DSWITCH="-DUSE_SCOREP"

make clean
rm -rf miniMD_openmpi miniMD_openmpi_ptf
make openmpi
mv miniMD_openmpi miniMD_openmpi_ptf
