#!/bin/sh

echo
echo "*** Running Gen-Vx-Mask to generate a polyline mask file for the Continental United States ***"
gen_vx_mask \
   ../data/sample_fcst/2005080700/wrfprs_ruc13_24.tm00_G212 \
   $MET_BASE/poly/CONUS.poly \
   ${TEST_OUT_DIR}/gen_vx_mask/CONUS_poly.nc -v 2

echo
echo "*** Running Gen-Vx-Mask to generate a circle mask file ***"
gen_vx_mask \
   ../data/sample_fcst/2005080700/wrfprs_ruc13_24.tm00_G212 \
   $MET_BASE/poly/MLB_Stadiums.txt \
   ${TEST_OUT_DIR}/gen_vx_mask/CONUS_circles.nc \
   -type circle -thresh le150 -v 2
