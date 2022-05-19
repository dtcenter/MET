


########################################################################


PLOT_POINT_OBS_EXEC = ${OTHER_DIR}/plot_point_obs/plot_point_obs


########################################################################


   ##
   ##  plot_point_obs
   ##
   ##     prerequisites: pb2nc
   ##


plot_point_obs: ${PLOT_POINT_OBS_EXEC} pb2nc
	@ echo
	@ echo "*** Running PLOT_POINT_OBS on a sample netCDF observation file ***"
	${PLOT_POINT_OBS_EXEC} \
   	${TEST_OUT_DIR}/pb2nc/sample_pb.nc \
   	${TEST_OUT_DIR}/plot_point_obs/sample_pb.ps
	@ 
	@ touch plot_point_obs


########################################################################


