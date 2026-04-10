mkdir -p  /d1/personal/johnhg/METplus/development/METplus-13.0/beta2/MET-feature_3294_eas_reduce_memory/test_output/gen_ens_prod
export 'CLIMO_MEAN_FILE=${MET_TEST_INPUT}/climatology_data/NCEP_1.0deg/cmean_1d.19790410'
export 'CLIMO_STDEV_FILE=${MET_TEST_INPUT}/climatology_data/NCEP_1.0deg/cstdv_1d.19790410'
echo "/d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep1/arw-fer-gep1_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep2/arw-sch-gep2_2012040912_F024.grib \
                MISSING \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nmm-fer-gep4/nmm-fer-gep4_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-fer-gep5/arw-fer-gep5_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-sch-gep6/arw-sch-gep6_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/arw-tom-gep7/arw-tom-gep7_2012040912_F024.grib \
                /d1/projects/MET/MET_test_data/unit_test/model_data/grib1/nmm-fer-gep8/nmm-fer-gep8_2012040912_F024.grib" \
                > /d1/personal/johnhg/METplus/development/METplus-13.0/beta2/MET-feature_3294_eas_reduce_memory/internal/test_unit/../../test_output/gen_ens_prod/input_file_list; \
          /d1/personal/johnhg/METplus/development/METplus-13.0/beta2/MET-feature_3294_eas_reduce_memory/internal/test_unit/../../share/met/../../bin/gen_ens_prod \
      -ens    /d1/personal/johnhg/METplus/development/METplus-13.0/beta2/MET-feature_3294_eas_reduce_memory/internal/test_unit/../../test_output/gen_ens_prod/input_file_list \
      -config /d1/personal/johnhg/METplus/development/METplus-13.0/beta2/MET-feature_3294_eas_reduce_memory/internal/test_unit/config/GenEnsProdConfig \
      -out    /d1/personal/johnhg/METplus/development/METplus-13.0/beta2/MET-feature_3294_eas_reduce_memory/internal/test_unit/../../test_output/gen_ens_prod/gen_ens_prod_NO_CTRL_20120410_120000V.nc \
      -v 4 -log eas.log 
