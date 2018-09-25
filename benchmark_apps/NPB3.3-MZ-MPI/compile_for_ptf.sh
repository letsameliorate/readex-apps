#!/bin/bash

#source env/set_env_npb_ptf.sh
source readex_env/set_env_ptf_hdeem.source

# Check environment
module list

#Compile NPB
cp config/make.def.ptf config/make.def

make bt-mz CLASS=C NPROCS=2

mv bin/bt-mz.C.2 bin/bt-mz.C.2_ptf
