#!/bin/sh

echo
echo "*** Running MODE on APCP using netCDF input for both forecast and observation ***"
mode \
   ${TEST_OUT_DIR}/pcp_combine/sample_fcst_12L_2005080712V_12A.nc \
   ${TEST_OUT_DIR}/pcp_combine/sample_obs_2005080712V_12A.nc \
   config/MODEConfig_APCP_12 \
   -outdir ${TEST_OUT_DIR}/mode -v 2

echo
echo "*** Running MODE on APCP using a GRIB forecast and netCDF observation ***"
mode \
   ../data/sample_fcst/2005080700/wrfprs_ruc13_24.tm00_G212 \
   ${TEST_OUT_DIR}/pcp_combine/sample_obs_2005080800V_24A.nc \
   config/MODEConfig_APCP_24 \
   -outdir ${TEST_OUT_DIR}/mode -v 2

echo
echo "*** Running MODE on RH at 500mb using GRIB input for both forecast and observation ***"   
mode \
   ../data/sample_fcst/2005080700/wrfprs_ruc13_12.tm00_G212 \
   ../data/sample_fcst/2005080712/wrfprs_ruc13_00.tm00_G212 \
   config/MODEConfig_RH \
   -outdir ${TEST_OUT_DIR}/mode -v 2
