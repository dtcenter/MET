


########################################################################


STAT_ANALYSIS_EXEC = ${CORE_DIR}/stat_analysis/stat_analysis


########################################################################


   ##
   ##  stat_analysis
   ##
   ##     prerequisites: grid_stat wavelet_stat ensemble_stat
   ##


stat_analysis: ${STAT_ANALYSIS_EXEC} grid_stat wavelet_stat ensemble_stat
	touch stat_analysis


########################################################################


