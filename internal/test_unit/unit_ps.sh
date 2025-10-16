export 'BEG_DS=-1800'
export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib1/gfs/gfs_2012040900_F012_gNam.grib"'
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'END_DS=1800'
export 'MASK_POLY_FILE=${MET_TEST_INPUT}/model_data/grib1/nam/nam_2012040900_F012.grib {name=\"LAND\";level=\"L0\";}'
export 'OUTPUT_PREFIX=GRIB1_NAM_GDAS'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_PHYS \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset CLIMO_FILE
unset CONFIG_DIR
unset END_DS
unset MASK_POLY_FILE
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib1/gfs/gfs_2012040900_F012_gNam.grib"'
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'END_DS=1800'
export 'OUTPUT_PREFIX=GRIB1_NAM_GDAS_WINDS'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_WINDS \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset CLIMO_FILE
unset CONFIG_DIR
unset END_DS
unset OUTPUT_PREFIX


export 'BEG_DS=-300'
export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib1/gfs/gfs_2012040900_F012_gNam.grib"'
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'END_DS=300'
export 'OUTPUT_PREFIX=GRIB1_NAM_GDAS_MPR_OBTYPE'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_MPR_OBTYPE \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset CLIMO_FILE
unset CONFIG_DIR
unset END_DS
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib1/gfs/gfs_2012040900_F012_gNam.grib"'
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'END_DS=1800'
export 'OUTPUT_PREFIX=GRIB1_NAM_GDAS_MASK_SID'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_MASK_SID \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset CLIMO_FILE
unset CONFIG_DIR
unset END_DS
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib2/gfs/gfs_2012040900_F012_gNam.grib2"'
export 'END_DS=1800'
export 'MASK_POLY_FILE=${MET_TEST_INPUT}/model_data/grib2/nam/nam_2012040900_F012.grib2 {name=\"LAND\";level=\"L0\";}'
export 'OUTPUT_PREFIX=GRIB2_NAM_NDAS'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/nam/nam_2012040900_F012.grib2 \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/ndas.20120409.t12z.prepbufr.tm00.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_PHYS \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset CLIMO_FILE
unset END_DS
unset MASK_POLY_FILE
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'CLIMO_FILE='
export 'END_DS=1800'
export 'MASK_POLY_FILE=${MET_TEST_INPUT}/model_data/grib2/sref_mn/sref_mean_2012040821_F015.grib2 {name=\"TMP\";level=\"Z2\";} <280'
export 'OUTPUT_PREFIX=GRIB2_SREF_GDAS'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_mn/sref_mean_2012040821_F015.grib2 \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_PHYS \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset CLIMO_FILE
unset END_DS
unset MASK_POLY_FILE
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'END_DS=1800'
export 'FCST_FIELD_LEVEL=A3'
export 'FCST_FIELD_NAME=APCP'
export 'OBS_DICT=fcst'
export 'OUTPUT_PREFIX=GRIB1_NAM_TRMM'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/ascii2nc/trmm_2012040912_3hr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_APCP \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset END_DS
unset FCST_FIELD_LEVEL
unset FCST_FIELD_NAME
unset OBS_DICT
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'END_DS=1800'
export 'FCST_FIELD_LEVEL=A3'
export 'FCST_FIELD_NAME=APCP'
export 'OBS_DICT=fcst'
export 'OUTPUT_PREFIX=GRIB2_SREF_TRMM'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_mn/sref_mean_2012040821_F012.grib2 \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/ascii2nc/trmm_2012040912_3hr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_APCP \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset END_DS
unset FCST_FIELD_LEVEL
unset FCST_FIELD_NAME
unset OBS_DICT
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'END_DS=1800'
export 'FCST_FIELD_LEVEL=(*,*)'
export 'FCST_FIELD_NAME=APCP_24'
export 'OBS_DICT={ field = [ { name  = "APCP"; level = "A24"; } ]; }'
export 'OUTPUT_PREFIX=NCMET_NAM_HMTGAGE'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/met_nc/nam/nam_2012040900_F036_APCP24.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_APCP \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset END_DS
unset FCST_FIELD_LEVEL
unset FCST_FIELD_NAME
unset OBS_DICT
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'END_DS=1800'
export 'FCST_FIELD_LEVEL=(*,*)'
export 'FCST_FIELD_NAME=APCP_24'
export 'OBS_DICT={ field = [ { name  = "TP24"; level = "L0"; is_precipitation = TRUE; } ]; }'
export 'OUTPUT_PREFIX=NCMET_NAM_NDAS_SEEPS'
export 'SEEPS_P1_THRESH=ge0.1&&le0.85'
export 'SEEPS_POINT_CLIMO_NAME='
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/met_nc/nam/nam_2012040900_F036_APCP24.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/ndas.20120410.t12z.prepbufr.tm00.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_SEEPS \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset END_DS
unset FCST_FIELD_LEVEL
unset FCST_FIELD_NAME
unset OBS_DICT
unset OUTPUT_PREFIX
unset SEEPS_P1_THRESH
unset SEEPS_POINT_CLIMO_NAME


export 'BEG_DS=-1800'
export 'END_DS=150000000'
export 'FCST_FIELD_LEVEL=(0,*,*)'
export 'FCST_FIELD_NAME=RAINNC'
export 'OBS_DICT={ field = [ { name = "APCP"; level = "A3"; } ]; }'
export 'OUTPUT_PREFIX=NCPINT_TRMM'
export 'SEEPS_FLAG=NONE'
export 'SEEPS_P1_THRESH=NA'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/p_interp/wrfout_d01_2008-08-08_12:00:00_PLEV \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/ascii2nc/trmm_2012040912_3hr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_APCP \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset END_DS
unset FCST_FIELD_LEVEL
unset FCST_FIELD_NAME
unset OBS_DICT
unset OUTPUT_PREFIX
unset SEEPS_FLAG
unset SEEPS_P1_THRESH


export 'BEG_DS=-1800'
export 'END_DS=150000000'
export 'OUTPUT_PREFIX=NCPINT_NDAS'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/p_interp/wrfout_d01_2008-08-08_12:00:00_PLEV \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/ndas.20120409.t12z.prepbufr.tm00.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_PHYS_pint \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset END_DS
unset OUTPUT_PREFIX


export 'OUTPUT_PREFIX=GRIB2_SREF_TRMM_prob'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_pr/sref_prob_2012040821_F015.grib2 \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/ascii2nc/trmm_2012040912_3hr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_prob \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 4
unset OUTPUT_PREFIX


export 'BEG_DS=-5400'
export 'END_DS=5400'
export 'OUTPUT_PREFIX=GTG_lc'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/nccf/gtg/lc/gtg_obs_forecast.20130827.i12.f03.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/ascii2nc/edr_hourly.20130827.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_GTG_lc \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 4
unset BEG_DS
unset END_DS
unset OUTPUT_PREFIX


export 'BEG_DS=-5400'
export 'END_DS=5400'
export 'OUTPUT_PREFIX=GTG_latlon'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/nccf/gtg/latlon/gtg_obs_forecast.20130827.i12.f06.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/ascii2nc/edr_hourly.20130827.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_GTG_latlon \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 4
unset BEG_DS
unset END_DS
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'CENSOR_THRESH='
export 'CENSOR_VAL='
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'END_DS=1800'
export 'OUTPUT_PREFIX=SID_INC_EXC'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_sid_inc_exc \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset CENSOR_THRESH
unset CENSOR_VAL
unset CONFIG_DIR
unset END_DS
unset OUTPUT_PREFIX


export 'BEG_DS=-1800'
export 'CENSOR_THRESH=lt-3.0, gt3.0'
export 'CENSOR_VAL=  -3.0,   3.0'
export 'CONFIG_DIR=${MET_TEST_BASE}/config'
export 'END_DS=1800'
export 'OUTPUT_PREFIX=SID_INC_EXC_CENSOR'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_sid_inc_exc \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset BEG_DS
unset CENSOR_THRESH
unset CENSOR_VAL
unset CONFIG_DIR
unset END_DS
unset OUTPUT_PREFIX


export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib1/gfs/gfs_2012040900_F012_gNam.grib"'
export 'OUTPUT_PREFIX=GRIB1_NAM_GDAS_INTERP_OPTS'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_INTERP_OPTS \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset CLIMO_FILE
unset OUTPUT_PREFIX


export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib1/gfs/gfs_2012040900_F012_gNam.grib"'
export 'OUTPUT_PREFIX=GRIB1_NAM_GDAS_INTERP_OPTS_name'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/ascii2nc/gdas1.20120409.t12z.prepbufr.ascii_name.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_INTERP_OPTS \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 1
unset CLIMO_FILE
unset OUTPUT_PREFIX


export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib1/gfs/gfs_2012040900_F012_gNam.grib"'
export 'GEOG_FILE="${MET_TEST_INPUT}/model_data/grib1/nam/nam_2012040900_F012.grib"'
export 'OUTPUT_PREFIX=LAND_TOPO_MASK'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_LAND_TOPO_MASK \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 2
unset CLIMO_FILE
unset GEOG_FILE
unset OUTPUT_PREFIX


export 'CLIMO_FILE="${MET_TEST_INPUT}/model_data/grib1/gfs/gfs_2012040900_F012_gNam.grib"'
export 'GEOG_FILE="${MET_TEST_INPUT}/model_data/grib1/nam/nam_2012040900_F012.grib"'
export 'OUTPUT_PREFIX=LAPSE_RATE_MSL_AGL'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nam/nam_2012040900_F012.grib \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_LAPSE_RATE_MSL_AGL \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 2
unset CLIMO_FILE
unset GEOG_FILE
unset OUTPUT_PREFIX


export 'CLIMO_MEAN_FILE_LIST="${MET_TEST_INPUT}/climatology_data/NCEP_NCAR_40YR_1.0deg/cmean_1d.19590409"
         '
export 'CLIMO_STDEV_FILE_LIST="${MET_TEST_INPUT}/climatology_data/NCEP_NCAR_40YR_1.0deg/cstdv_1d.19590409"
         '
export 'DAY_INTERVAL=1'
export 'HOUR_INTERVAL=6'
export 'OUTPUT_PREFIX=MPR_THRESH'
/d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/gfs/gfs_2012040900_F012.grib2 \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/pb2nc/ndas.20120409.t12z.prepbufr.tm00.nc \
      /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/config/PointStatConfig_mpr_thresh \
      -outdir /d1/personal/johnhg/MET/MET_development/met-12.2.0/rc2/MET-feature_3258_main_v12.2_lapse_rate/internal/test_unit/../../test_output/point_stat -v 3
unset CLIMO_MEAN_FILE_LIST
unset CLIMO_STDEV_FILE_LIST
unset DAY_INTERVAL
unset HOUR_INTERVAL
unset OUTPUT_PREFIX


