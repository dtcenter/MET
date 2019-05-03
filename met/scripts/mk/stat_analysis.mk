


########################################################################


STAT_ANALYSIS_EXEC = ${CORE_DIR}/stat_analysis/stat_analysis


########################################################################


   ##
   ##  stat_analysis
   ##
   ##     prerequisites: grid_stat wavelet_stat ensemble_stat
   ##


stat_analysis: ${STAT_ANALYSIS_EXEC} grid_stat wavelet_stat ensemble_stat
	@ echo
	@ echo "*** Running STAT-Analysis ***"
	${STAT_ANALYSIS_EXEC} \
   	-config config/STATAnalysisConfig \
   	-lookin ${TEST_OUT_DIR}/grid_stat \
   	-lookin ${TEST_OUT_DIR}/wavelet_stat \
   	-lookin ${TEST_OUT_DIR}/ensemble_stat \
   	-out ${TEST_OUT_DIR}/stat_analysis/stat_analysis.out \
   	-v 2
	@ 
	@ touch stat_analysis


########################################################################


