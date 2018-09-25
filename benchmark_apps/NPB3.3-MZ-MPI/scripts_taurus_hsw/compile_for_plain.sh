#!/bin/bash
cd ..

#source env/set_env_npb_plain.sh
source readex_env/set_env_plain.source

# Check environment
module list

#Compile NPB
cp config/make.def.plain config/make.def
CLASS=C
NPROCS=2
make bt-mz CLASS=${CLASS} NPROCS=${NPROCS}

mv bin/bt-mz.${CLASS}.${NPROCS} bin/bt-mz.${CLASS}.${NPROCS}_plain
