


########################################################################


GRID_STAT_EXEC = ${CORE_DIR}/grid_stat/grid_stat


########################################################################


   ##
   ##  grid_stat
   ##
   ##     prerequisites:  pcp_combine
   ##


grid_stat: ${GRID_STAT_EXEC} pcp_combine
	touch grid_stat


########################################################################


