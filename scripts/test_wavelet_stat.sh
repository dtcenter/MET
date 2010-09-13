#!/bin/sh

echo
echo "*** Running Wavelet-Stat on APCP using a GRIB forecast and netCDF observation ***"
../bin/wavelet_stat \
   ../data/sample_fcst/2005080700/wrfprs_ruc13_12.tm00_G212 \
   ../out/pcp_combine/sample_obs_2005080712V_12A.nc \
   config/WaveletStatConfig_APCP_12 \
   -outdir ../out/wavelet_stat -v 2

echo
echo "*** Running Wavelet-Stat on APCP using netCDF input for both forecast and observation ***"
../bin/wavelet_stat \
   ../out/pcp_combine/sample_fcst_12L_2005080800V_12A.nc \
   ../out/pcp_combine/sample_obs_2005080800V_12A.nc \
   config/WaveletStatConfig_APCP_12_NC \
   -outdir ../out/wavelet_stat -v 2
