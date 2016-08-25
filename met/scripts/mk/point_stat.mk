


########################################################################


POINT_STAT_EXEC = ${CORE_DIR}/point_stat/point_stat


########################################################################


   ##
   ##  point_stat
   ##
   ##     prerequisites: pb2nc ascii2nc
   ##


point_stat: ${POINT_STAT_EXEC} pb2nc ascii2nc
	touch point_stat


########################################################################


