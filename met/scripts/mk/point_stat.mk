


########################################################################


POINT_STAT_EXEC = ${CORE_DIR}/point_stat/point_stat


########################################################################


   ##
   ##  point_stat
   ##
   ##     prerequisites: pb2nc ascii2nc
   ##


point_stat: ${POINT_STAT_EXEC} pb2nc ascii2nc
	@ echo
	@ echo "*** Running Point-Stat on sample NAM data ***"
	${POINT_STAT_EXEC} \
   	../data/sample_fcst/2007033000/nam.t00z.awip1236.tm00.20070330.grb \
   	${TEST_OUT_DIR}/pb2nc/sample_pb.nc \
   	config/PointStatConfig \
   	-point_obs ${TEST_OUT_DIR}/ascii2nc/sample_ascii.nc \
   	-outdir ${TEST_OUT_DIR}/point_stat -v 2
	@ 
	@ touch point_stat


########################################################################


