execute_process(COMMAND ${ELMERGRID_BIN} 1 2 global)
execute_process(COMMAND ${ELMERGRID_BIN} 1 2 local)
execute_process(COMMAND ${ELMERGRID_BIN} 1 2 reference)
RUN_ELMER_TEST()