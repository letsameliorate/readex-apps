#!/bin/sh

source ../../init.sh
rm ${REL_RDD_CONFIG_FILE_NAME}
cd ${REL_PATH_APP_EXECUTION}

#module purge
#module use /projects/p_readex/modules
#module load readex/ci_readex_bullxmpi1.2.8.4_gcc6.3.0

rm miniMD_openmpi_rdd
sh compile_for_rdd_manual.sh
