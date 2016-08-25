


########################################################################


PLOT_POINT_OBS_EXEC = ${OTHER_DIR}/plot_point_obs/plot_point_obs


########################################################################


   ##
   ##  plot_point_obs
   ##
   ##     prerequisites: pb2nc
   ##


plot_point_obs: ${PLOT_POINT_OBS_EXEC} pb2nc
	touch plot_point_obs


########################################################################


