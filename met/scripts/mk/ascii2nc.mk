


########################################################################


ASCII2NC_EXEC = ${OTHER_DIR}/ascii2nc/ascii2nc


########################################################################


   ##
   ##  ascii2nc
   ##
   ##     prerequisites:
   ##


ascii2nc: ${ASCII2NC_EXEC}
	@ echo
	@ echo "*** Running ASCII2NC to reformat ASCII point observations into NetCDF ***"
	${ASCII2NC_EXEC} \
   	../data/sample_obs/ascii/sample_ascii_obs.txt \
   	${TEST_OUT_DIR}/ascii2nc/sample_ascii.nc \
   	-v 2
	@
	@ echo
	@ echo "*** Running ASCII2NC to reformat ASCII rain gauge observations into NetCDF ***"
	${ASCII2NC_EXEC} \
   	../data/sample_obs/ascii/precip24_2010010112.ascii \
   	${TEST_OUT_DIR}/ascii2nc/precip24_2010010112.nc \
   	-v 2
	@
	@ touch ascii2nc


########################################################################


