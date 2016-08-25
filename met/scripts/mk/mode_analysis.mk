


########################################################################


MODE_ANALYSIS_EXEC = ${CORE_DIR}/mode_analysis/mode_analysis


########################################################################


   ##
   ##  mode_analysis
   ##
   ##     prerequisites: mode
   ##


mode_analysis: ${MODE_ANALYSIS_EXEC} mode
	touch mode_analysis


########################################################################


