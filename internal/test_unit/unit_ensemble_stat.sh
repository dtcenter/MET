export 'DESC=NA'
export 'OBS_ERROR_FLAG=FALSE'
export 'OUTPUT_PREFIX=CMD_LINE'
export 'SKIP_CONST=FALSE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      6 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/obs_data/laps/laps_2012041012_F000.grib \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OBS_ERROR_FLAG
unset OUTPUT_PREFIX
unset SKIP_CONST


export 'DESC=NA'
export 'OBS_ERROR_FLAG=FALSE'
export 'OUTPUT_PREFIX=FILE_LIST'
export 'SKIP_CONST=FALSE'
echo "/d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib" \
                > /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat/input_file_list; \
          /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat/input_file_list \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/obs_data/laps/laps_2012041012_F000.grib \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OBS_ERROR_FLAG
unset OUTPUT_PREFIX
unset SKIP_CONST


export 'CENSOR_THRESH='
export 'CENSOR_VAL='
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'OUTPUT_PREFIX=MASK_SID'
export 'SKIP_CONST=FALSE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      6 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_MASK_SID \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset CENSOR_THRESH
unset CENSOR_VAL
unset CONFIG_DIR
unset OUTPUT_PREFIX
unset SKIP_CONST


export 'CENSOR_THRESH='
export 'CENSOR_VAL='
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'OUTPUT_PREFIX=MASK_SID_CTRL'
export 'SKIP_CONST=FALSE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      5 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_MASK_SID \
      -ctrl /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset CENSOR_THRESH
unset CENSOR_VAL
unset CONFIG_DIR
unset OUTPUT_PREFIX
unset SKIP_CONST


export 'CENSOR_THRESH=lt0, gt5'
export 'CENSOR_VAL=  0,   5'
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'OUTPUT_PREFIX=MASK_SID_CENSOR'
export 'SKIP_CONST=FALSE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      6 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_MASK_SID \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset CENSOR_THRESH
unset CENSOR_VAL
unset CONFIG_DIR
unset OUTPUT_PREFIX
unset SKIP_CONST


export 'DESC=SKIP_CONST'
export 'OBS_ERROR_FLAG=FALSE'
export 'OUTPUT_PREFIX=SKIP_CONST'
export 'SKIP_CONST=TRUE'
echo "/d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib" \
                > /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat/input_file_list; \
          /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat/input_file_list \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/obs_data/laps/laps_2012041012_F000.grib \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OBS_ERROR_FLAG
unset OUTPUT_PREFIX
unset SKIP_CONST


export 'DESC=OBSERR'
export 'OBS_ERROR_FLAG=TRUE'
export 'OUTPUT_PREFIX=OBSERR'
export 'SKIP_CONST=TRUE'
echo "/d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib" \
                > /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat/input_file_list; \
          /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat/input_file_list \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/obs_data/laps/laps_2012041012_F000.grib \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OBS_ERROR_FLAG
unset OUTPUT_PREFIX
unset SKIP_CONST


export 'DESC=OBSERR_BAD_LOOKUP'
export 'MET_OBS_ERROR_TABLE=${MET_TEST_OUTPUT}/ensemble_stat/obs_error_table_truncated.txt'
export 'OBS_ERROR_FLAG=TRUE'
export 'OUTPUT_PREFIX=OBSERR_BAD_LOOKUP'
export 'SKIP_CONST=TRUE'
grep -E "OBS_VAR|APCP" /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/table_files/obs_error_table.txt | head -2 \
          > /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat/obs_error_table_truncated.txt; \
          /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat/input_file_list \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/obs_data/laps/laps_2012041012_F000.grib \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset MET_OBS_ERROR_TABLE
unset OBS_ERROR_FLAG
unset OUTPUT_PREFIX
unset SKIP_CONST


export 'DESC=SINGLE_FILE_NC_NO_CTRL'
export 'OUTPUT_PREFIX=SINGLE_FILE_NC_NO_CTRL'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      1 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/CPC_NMME/CFSv2.tmp2m.198201.fcst.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_single_file_nc \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/model_data/CPC_NMME/CFSv2.tmp2m.198201.fcst.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OUTPUT_PREFIX


export 'DESC=SINGLE_FILE_NC_WITH_CTRL'
export 'OUTPUT_PREFIX=SINGLE_FILE_NC_WITH_CTRL'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      1 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/CPC_NMME/CFSv2.tmp2m.198201.fcst.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_single_file_nc \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/model_data/CPC_NMME/CFSv2.tmp2m.198201.fcst.nc \
      -ctrl /d1/projects/MET/MET_test_data/unit_test/model_data/CPC_NMME/CFSv2.tmp2m.198201.fcst.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OUTPUT_PREFIX


export 'DESC=SINGLE_FILE_GRIB_NO_CTRL'
export 'OUTPUT_PREFIX=SINGLE_FILE_GRIB_NO_CTRL'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      1 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/gefs/enspost_grb2.t00z.prmsl \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_single_file_grib \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/gefs/enspost_grb2.t00z.prmsl \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OUTPUT_PREFIX


export 'DESC=SINGLE_FILE_GRIB_WITH_CTRL'
export 'OUTPUT_PREFIX=SINGLE_FILE_GRIB_WITH_CTRL'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      1 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/gefs/enspost_grb2.t00z.prmsl \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_single_file_grib \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/gefs/enspost_grb2.t00z.prmsl \
      -ctrl /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/gefs/enspost_grb2.t00z.prmsl \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OUTPUT_PREFIX


export 'DESC=RPS_CLIMO_BIN_PROB'
export 'OUTPUT_PREFIX=RPS_CLIMO_BIN_PROB'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      1 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/cpc_climo_prob/gefs-00z_aer-rfcst-cal_tmean_20210101_week2_cats_33-67ptile.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_rps_climo_bin_prob \
      -grid_obs /d1/projects/MET/MET_test_data/unit_test/model_data/cpc_climo_prob/tmean_07d_20210115_cats_33-67ptile.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 1
unset DESC
unset OUTPUT_PREFIX


export 'GEOG_FILE="${MET_TEST_INPUT}/model_data/grib1/nam/nam_2012040900_F012.grib"'
export 'OUTPUT_PREFIX=LAND_TOPO_MASK'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      6 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040900_F012.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_LAND_TOPO_MASK \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 2
unset GEOG_FILE
unset OUTPUT_PREFIX


export 'GEOG_FILE="${MET_TEST_INPUT}/model_data/grib1/nam/nam_2012040900_F012.grib"'
export 'OUTPUT_PREFIX=LAPSE_RATE_MSL_AGL'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      6 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040900_F012.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040900_F012.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/config/EnsembleStatConfig_LAPSE_RATE_MSL_AGL \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      -point_obs 'PYTHON_NUMPY=/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../share/met/python/examples/read_ascii_point.py /d1/projects/MET/MET_test_data/unit_test/obs_data/ascii/sample_ascii_KMUO_BAD_ELEVATION.txt' \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-bugfix_3429_develop_obs_error_table_lookup/internal/test_unit/../../test_output/ensemble_stat -v 2
unset GEOG_FILE
unset OUTPUT_PREFIX


