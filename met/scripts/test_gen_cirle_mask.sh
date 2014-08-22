#!/bin/sh

echo
echo "*** Running Gen-Circle-Mask to generate a polyline mask file for the Continental United States ***"
gen_circle_mask \
   ../data/sample_fcst/2005080700/wrfprs_ruc13_24.tm00_G212 \
   $MET_BASE/poly/MLB_Stadiums.txt \
   ${TEST_OUT_DIR}/gen_circle_mask/CONUS_circles.nc \
   -radius 150 -v 2
