#/bin/sh

echo
echo "*** Running Gen-Ens-Prod using GRIB forecasts ***"
gen_ens_prod \
   -ens ../data/sample_fcst/2009123112/*gep*/d01_2009123112_02400.grib \
   -config config/GenEnsProdConfig \
   -out ${TEST_OUT_DIR}/gen_ens_prod/gen_ens_prod_20100101_120000V_ens.nc -v 2
