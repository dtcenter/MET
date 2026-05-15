export 'DESC=NO_WEIGHT'
export 'MASK_GRID="FULL"'
export 'MASK_SID="${MET_TEST_BASE}/config/SID_CONUS_ADPUPA_ELEV.txt"'
export 'OUTPUT_PREFIX=NO_WEIGHT'
export 'POINT_WEIGHT=NONE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_mn/sref_mean_2012040821_F015.grib2 \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/PointStatConfig_point_weight \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


export 'DESC=SID_WEIGHT'
export 'MASK_GRID='
export 'MASK_SID="${MET_TEST_BASE}/config/SID_CONUS_ADPUPA_ELEV.txt"'
export 'OUTPUT_PREFIX=SID_WEIGHT'
export 'POINT_WEIGHT=SID'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_mn/sref_mean_2012040821_F015.grib2 \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/PointStatConfig_point_weight \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


export 'DESC=KDE_WEIGHT'
export 'MASK_GRID="FULL"'
export 'MASK_SID='
export 'OUTPUT_PREFIX=KDE_WEIGHT'
export 'POINT_WEIGHT=KDE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_mn/sref_mean_2012040821_F015.grib2 \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/PointStatConfig_point_weight \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


export 'DESC=NO_WEIGHT'
export 'MASK_GRID="FULL"'
export 'MASK_SID="${MET_TEST_BASE}/config/SID_CONUS_ADPUPA_ELEV.txt"'
export 'OUTPUT_PREFIX=PROB_NO_WEIGHT'
export 'POINT_WEIGHT=NONE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_pr/sref_prob_2012040821_F015.grib2 \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/PointStatConfig_prob_point_weight \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


export 'DESC=SID_WEIGHT'
export 'MASK_GRID='
export 'MASK_SID="${MET_TEST_BASE}/config/SID_CONUS_ADPUPA_ELEV.txt"'
export 'OUTPUT_PREFIX=PROB_SID_WEIGHT'
export 'POINT_WEIGHT=SID'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_pr/sref_prob_2012040821_F015.grib2 \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/PointStatConfig_prob_point_weight \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


export 'DESC=KDE_WEIGHT'
export 'MASK_GRID="FULL"'
export 'MASK_SID='
export 'OUTPUT_PREFIX=PROB_KDE_WEIGHT'
export 'POINT_WEIGHT=KDE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/point_stat \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib2/sref_pr/sref_prob_2012040821_F015.grib2 \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/pb2nc/gdas1.20120409.t12z.prepbufr.nc \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/PointStatConfig_prob_point_weight \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


export 'DESC=NO_WEIGHT'
export 'MASK_GRID="FULL"'
export 'MASK_SID="${MET_TEST_BASE}/config/SID_CONUS_ADPUPA_ELEV.txt"'
export 'OUTPUT_PREFIX=NO_WEIGHT'
export 'POINT_WEIGHT=NONE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      6 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/EnsembleStatConfig_point_weight \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


export 'DESC=SID_WEIGHT'
export 'MASK_GRID='
export 'MASK_SID="${MET_TEST_BASE}/config/SID_CONUS_ADPSFC_ELEV.txt"'
export 'OUTPUT_PREFIX=SID_WEIGHT'
export 'POINT_WEIGHT=SID'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      6 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/EnsembleStatConfig_point_weight \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


export 'DESC=KDE_WEIGHT'
export 'MASK_GRID="FULL"'
export 'MASK_SID='
export 'OUTPUT_PREFIX=KDE_WEIGHT'
export 'POINT_WEIGHT=KDE'
/d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../share/met/../../bin/ensemble_stat \
      6 \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep3/arw-tom-gep3_2012040912_F024.grib \
      /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib \
      /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/config/EnsembleStatConfig_point_weight \
      -point_obs /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/ascii2nc/gauge_2012041012_24hr.nc \
      -outdir /d1/personal/johnhg/METplus/development/METplus-13.0/rc1/MET-feature_3335_point_weight_flag/internal/test_unit/../../test_output/point_weight -v 1
unset DESC
unset MASK_GRID
unset MASK_SID
unset OUTPUT_PREFIX
unset POINT_WEIGHT


