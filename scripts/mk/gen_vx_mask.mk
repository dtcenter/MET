


########################################################################


GEN_VX_MASK_EXEC = ${OTHER_DIR}/gen_vx_mask/gen_vx_mask


########################################################################


   ##
   ##  gen_vx_mask
   ##
   ##     prerequisites:
   ##


gen_vx_mask: ${GEN_VX_MASK_EXEC}
	echo
	echo "*** Running Gen-Vx-Mask to generate a polyline mask file for the Continental United States ***"
	${GEN_VX_MASK_EXEC} \
   	../data/sample_fcst/2005080700/wrfprs_ruc13_24.tm00_G212 \
	../data/poly/CONUS.poly \
   	${TEST_OUT_DIR}/gen_vx_mask/CONUS_poly.nc -v 2
	@ 
	echo
	echo "*** Running Gen-Vx-Mask to generate a circle mask file ***"
	${GEN_VX_MASK_EXEC} \
   	../data/sample_fcst/2005080700/wrfprs_ruc13_24.tm00_G212 \
	../data/poly/MLB_Stadiums.txt \
   	${TEST_OUT_DIR}/gen_vx_mask/CONUS_circles.nc \
	 -type circle -thresh le150 -v 2
	@ 
	@ touch gen_vx_mask


########################################################################


