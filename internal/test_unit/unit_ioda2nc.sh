export 'MASK_GRID='
export 'MASK_POLY='
export 'MESSAGE_TYPE='
export 'QUALITY_MARK_THRESH=NA'
export 'STATION_ID="KEKA"'
/d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../share/met/../../bin/ioda2nc \
      /d1/projects/MET/MET_test_data/unit_test/obs_data/ioda/ioda.NC001007.2020031012.nc \
      /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../test_output/ioda2nc/ioda.NC001007.2020031012.mask_sid.nc \
      -config /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/config/IODA2NCConfig_mask \
      -v 2
unset MASK_GRID
unset MASK_POLY
unset MESSAGE_TYPE
unset QUALITY_MARK_THRESH
unset STATION_ID


/d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../share/met/../../bin/ioda2nc \
      /d1/projects/MET/MET_test_data/unit_test/obs_data/ioda/odb_sonde_16019.nc4 \
      /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../test_output/ioda2nc/odb_sonde_16019_all.nc \
      -v 2


/d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../share/met/../../bin/ioda2nc \
      /d1/projects/MET/MET_test_data/unit_test/obs_data/ioda/ioda.NC001007.2020031012.nc \
      /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../test_output/ioda2nc/ioda.NC001007.2020031012.summary.nc \
      -config /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/config/IODA2NCConfig_summary \
      -v 2


export 'MASK_GRID='
export 'MASK_POLY='
export 'MESSAGE_TYPE='
export 'QUALITY_MARK_THRESH=NA'
export 'STATION_ID="KEKA"'
/d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../share/met/../../bin/ioda2nc \
      /d1/projects/MET/MET_test_data/unit_test/obs_data/ioda/ioda.NC001007.2020031012.nc \
      /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../test_output/ioda2nc/ioda.NC001007.2020031012.same_input.nc \
      -config /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/config/IODA2NCConfig_mask \
      -iodafile /d1/projects/MET/MET_test_data/unit_test/obs_data/ioda/ioda.NC001007.2020031012.nc \
      -v 2
unset MASK_GRID
unset MASK_POLY
unset MESSAGE_TYPE
unset QUALITY_MARK_THRESH
unset STATION_ID


export 'MASK_GRID='
export 'MASK_POLY='
export 'MESSAGE_TYPE='
export 'QUALITY_MARK_THRESH=le64'
export 'STATION_ID='
/d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../share/met/../../bin/ioda2nc \
      /d1/projects/MET/MET_test_data/unit_test/obs_data/ioda/jopa_satwind_20210701T1200Z_out_0000_reduced.nc4 \
      /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../test_output/ioda2nc/jopa_satwind_20210701T1200Z_int_datetime.nc \
      -config /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/config/IODA2NCConfig_mask \
      -v 2
unset MASK_GRID
unset MASK_POLY
unset MESSAGE_TYPE
unset QUALITY_MARK_THRESH
unset STATION_ID


export 'MASK_GRID='
export 'MASK_POLY='
export 'MESSAGE_TYPE='
export 'QUALITY_MARK_THRESH=NA'
export 'STATION_ID='
/d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../share/met/../../bin/ioda2nc \
      /d1/projects/MET/MET_test_data/unit_test/obs_data/ioda/2021081612_sonde_small.nc \
      /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/../../test_output/ioda2nc/2021081612_sonde_small_sid.nc \
      -config /d1/personal/johnhg/MET/MET_development/met-13.0.0/beta1/MET-feature_3307_ioda2nc/internal/test_unit/config/IODA2NCConfig_mask \
      -v 2
unset MASK_GRID
unset MASK_POLY
unset MESSAGE_TYPE
unset QUALITY_MARK_THRESH
unset STATION_ID


