


########################################################################


MADIS2NC_EXEC = ${OTHER_DIR}/madis2nc/madis2nc


########################################################################


   ##
   ##  madis2nc
   ##
   ##     prerequisites:
   ##


madis2nc: ${MADIS2NC_EXEC}
	@ echo
	@ echo "*** Running MADIS2NC to reformat MADIS point observations into NetCDF ***"
	${MADIS2NC_EXEC} \
  	../data/sample_obs/madis/metar/metar_2012040912_F000.nc \
  	${TEST_OUT_DIR}/madis2nc/metar_2012040912_F000.nc \
  	-type metar -rec_beg 2000 -rec_end 6000 -v 2
	@ 
	@ touch madis2nc


########################################################################


