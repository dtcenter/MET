


########################################################################


GEN_ENS_PROD_EXEC = ${OTHER_DIR}/gen_ens_prod/gen_ens_prod


########################################################################


   ##
   ##  gen_ens_prod
   ##
   ##     prerequisites:
   ##


gen_ens_prod: ${GEN_ENS_PROD_EXEC}
	@ echo
	@ echo "*** Running Gen-Ens-Prod using GRIB forecasts ***"
	${GEN_ENS_PROD_EXEC} \
   	-ens ../data/sample_fcst/2009123112/*gep*/d01_2009123112_02400.grib \
   	-config config/GenEnsProdConfig \
   	-out ${TEST_OUT_DIR}/gen_ens_prod/gen_ens_prod_20100101_120000V_ens.nc -v 2
	@
	@ touch gen_ens_prod


########################################################################


