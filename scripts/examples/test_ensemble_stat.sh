#/bin/sh

echo
echo "*** Running Ensemble-Stat on APCP using GRIB forecasts, point observations, and gridded observations ***"
ensemble_stat \
   6 ../data/sample_fcst/2009123112/*gep*/d01_2009123112_02400.grib \
   config/EnsembleStatConfig \
   -grid_obs ../data/sample_obs/ST4/ST4.2010010112.24h \
   -point_obs ${TEST_OUT_DIR}/ascii2nc/precip24_2010010112.nc \
   -outdir ${TEST_OUT_DIR}/ensemble_stat -v 2
