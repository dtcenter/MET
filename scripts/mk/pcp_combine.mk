


########################################################################


PCP_COMBINE_EXEC = ${CORE_DIR}/pcp_combine/pcp_combine


########################################################################


   ##
   ##  pcp combine
   ##
   ##     prerequisites: none
   ##


pcp_combine: ${PCP_COMBINE_EXEC}
	@ echo
	@ echo "*** Running PCP-Combine to combine 3-hourly APCP accumulation forecasts into a single 12 hour accumulation forecast ***"
	${PCP_COMBINE_EXEC} \
   	20050807_000000 3 20050807_120000 12 \
   	${TEST_OUT_DIR}/pcp_combine/sample_fcst_12L_2005080712V_12A.nc \
	-pcpdir ../data/sample_fcst/2005080700
	@
	@ echo
	@ echo "*** Running PCP-Combine to combine 3-hourly APCP accumulation forecasts into a single 12 hour accumulation forecast ***"
	${PCP_COMBINE_EXEC} \
   	20050807_000000 3 20050808_000000 12 \
   	${TEST_OUT_DIR}/pcp_combine/sample_fcst_12L_2005080800V_12A.nc \
   	-pcpdir ../data/sample_fcst/2005080700
	@
	@ echo
	@ echo "*** Running PCP-Combine to combine 3-hourly APCP accumulation forecasts into a single 24 hour accumulation forecast ***"
	${PCP_COMBINE_EXEC} \
   	20050807_000000 3 20050807_240000 24 \
   	${TEST_OUT_DIR}/pcp_combine/sample_fcst_24L_2005080800V_24A.nc \
   	-pcpdir ../data/sample_fcst/2005080700
	@
	@ echo
	@ echo "*** Running PCP-Combine to combine 1-hourly APCP accumulation observations into a single 12 hour accumulation observation ***"
	${PCP_COMBINE_EXEC} \
   	00000000_000000 1 20050807_120000 12 \
   	${TEST_OUT_DIR}/pcp_combine/sample_obs_2005080712V_12A.nc \
   	-pcpdir ../data/sample_obs/ST2ml
	@
	@ echo
	@ echo "*** Running PCP-Combine to combine 1-hourly APCP accumulation observations into a single 12 hour accumulation observation ***"
	${PCP_COMBINE_EXEC} \
   	00000000_000000 1 20050808_000000 12 \
   	${TEST_OUT_DIR}/pcp_combine/sample_obs_2005080800V_12A.nc \
   	-pcpdir ../data/sample_obs/ST2ml
	@
	@ echo
	@ echo "*** Running PCP-Combine to combine 1-hourly APCP accumulation observations into a single 24 hour accumulation observation ***"
	${PCP_COMBINE_EXEC} \
   	00000000_000000 1 20050808_000000 24 \
   	${TEST_OUT_DIR}/pcp_combine/sample_obs_2005080800V_24A.nc \
	-pcpdir ../data/sample_obs/ST2ml
	@
	@ touch pcp_combine


########################################################################


