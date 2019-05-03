#!/bin/sh

echo
echo "*** Running Gen-Poly-Mask to generate a polyline mask file for the Continental United States ***"
gen_poly_mask \
   ../data/sample_fcst/2005080700/wrfprs_ruc13_24.tm00_G212 \
   $MET_BASE/poly/CONUS.poly \
   ${TEST_OUT_DIR}/gen_poly_mask/CONUS_poly.nc -v 2
