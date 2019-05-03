


########################################################################


MODE_ANALYSIS_EXEC = ${CORE_DIR}/mode_analysis/mode_analysis


########################################################################


   ##
   ##  mode_analysis
   ##
   ##     prerequisites: mode
   ##


mode_analysis: ${MODE_ANALYSIS_EXEC} mode
	@ echo
	@ echo "*** Running MODE-Analysis to compute column summaries for simple forecast objects ***"
	${MODE_ANALYSIS_EXEC} \
   	-lookin ${TEST_OUT_DIR}/mode/mode_120000L_20050807_120000V_120000A_obj.txt \
   	-summary \
   	-column AREA -column LENGTH -column WIDTH -column CURVATURE -column INTENSITY_50 \
   	-fcst -config config/MODEAnalysisConfig \
   	-dump_row ${TEST_OUT_DIR}/mode_analysis/job_summary_APCP_12_simple_fcst.txt \
   	-out      ${TEST_OUT_DIR}/mode_analysis/job_summary_APCP_12_simple_fcst.out \
   	-v 2
	@ 
	@ echo
	@ echo "*** Running MODE-Analysis to compute column summaries for simple observation objects ***"
	${MODE_ANALYSIS_EXEC} \
   	-lookin ${TEST_OUT_DIR}/mode/mode_120000L_20050807_120000V_120000A_obj.txt \
   	-summary \
   	-column AREA -column LENGTH -column WIDTH -column CURVATURE -column INTENSITY_50 \
   	-obs -config config/MODEAnalysisConfig \
   	-dump_row ${TEST_OUT_DIR}/mode_analysis/job_summary_APCP_12_simple_obs.txt \
   	-out      ${TEST_OUT_DIR}/mode_analysis/job_summary_APCP_12_simple_obs.out \
   	-v 2
	@ 
	@ echo
	@ echo "*** Running MODE-Analysis to summarize by case ***"
	${MODE_ANALYSIS_EXEC} \
   	-lookin ${TEST_OUT_DIR}/mode/mode_120000L_20050807_120000V_120000A_obj.txt \
   	-lookin ${TEST_OUT_DIR}/mode/mode_240000L_20050808_000000V_240000A_obj.txt \
   	-bycase -single -simple \
   	-dump_row ${TEST_OUT_DIR}/mode_analysis/job_summary_bycase.txt \
   	-out      ${TEST_OUT_DIR}/mode_analysis/job_summary_bycase.out \
   	-v 2
	@ 
	@ touch mode_analysis


########################################################################


