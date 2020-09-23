#!/bin/bash

# Check that required unit test environment variables are set
if [[ -z "${MET_BASE}"       ||
      -z "${MET_BUILD_BASE}" ||
      -z "${MET_TEST_BASE}"  ||
      -z "${MET_TEST_INPUT}" ||
      -z "${MET_TEST_OUTPUT}" ]]; then
  echo
  echo "ERROR: Required environment variable(s) unset:"
  echo "ERROR: \${MET_BASE}, \${MET_BUILD_BASE}, \${MET_TEST_BASE}, \${MET_TEST_INPUT}, \${MET_TEST_OUTPUT}"
  echo
  exit
else
  echo "export MET_BASE=${MET_BASE}"
  echo "export MET_BUILD_BASE=${MET_BUILD_BASE}"
  echo "export MET_TEST_BASE=${MET_TEST_BASE}"
  echo "export MET_TEST_INPUT=${MET_TEST_INPUT}"
  echo "export MET_TEST_OUTPUT=${MET_TEST_OUTPUT}"
fi

PERL_UNIT_OPTS=""
for arg in $@; do
  [ $arg == "-memchk" -o $arg == "memchk" ] && PERL_UNIT_OPTS="$PERL_UNIT_OPTS -memchk"
  [ $arg == "-callchk" -o $arg == "callchk" ] && PERL_UNIT_OPTS="$PERL_UNIT_OPTS -callchk"
done

# Unit test script
PERL_UNIT=${MET_TEST_BASE}/perl/unit.pl

# Unit test XML
UNIT_XML="unit_ascii2nc.xml \
          unit_madis2nc.xml \
          unit_trmm2nc.xml \
          unit_pb2nc.xml \
          unit_gen_vx_mask.xml \
          unit_pcp_combine.xml \
          unit_wwmca_regrid.xml \
          unit_point_stat.xml \
          unit_duplicate_flag.xml \
          unit_obs_summary.xml \
          unit_grid_stat.xml \
          unit_wavelet_stat.xml \
          unit_ensemble_stat.xml \
          unit_stat_analysis.xml \
          unit_mode.xml \
          unit_mode_analysis.xml \
          unit_plot_point_obs.xml \
          unit_plot_data_plane.xml \
          unit_wwmca_plot.xml \
          unit_series_analysis.xml \
          unit_tc_dland.xml \
          unit_tc_pairs.xml \
          unit_tc_stat.xml \
          unit_plot_tc.xml \
          unit_tc_rmw.xml \
          unit_rmw_analysis.xml \
          unit_tc_gen.xml \
          unit_met_test_scripts.xml  \
          unit_modis.xml  \
          unit_ref_config.xml \
          unit_mode_graphics.xml \
          unit_regrid.xml \
          unit_gsi_tools.xml \
          unit_aeronet.xml \
          unit_shift_data_plane.xml \
          unit_mtd.xml \
          unit_climatology.xml \
          unit_test_grib_tables.xml \
          unit_grid_weight.xml \
          unit_netcdf.xml \
          unit_hira.xml \
          unit_interp_shape.xml \
          unit_lidar2nc.xml \
          unit_airnow.xml \
          unit_python.xml \
          unit_point2grid.xml \
          unit_perc_thresh.xml \
          unit_gaussian.xml \
          unit_grid_diag.xml \
          unit_quality_filter.xml"

# Run each unit test
for CUR_XML in ${UNIT_XML}; do

  echo
  echo "CALLING: ${PERL_UNIT} $PERL_UNIT_OPTS ${MET_TEST_BASE}/xml/${CUR_XML}"
  echo
  ${PERL_UNIT} $PERL_UNIT_OPTS ${MET_TEST_BASE}/xml/${CUR_XML}
  RET_VAL=$?

  # Fail on non-zero return status
  if [ ${RET_VAL} != 0 ]; then
    echo
    echo "ERROR: ${PERL_UNIT} ${CUR_XML} failed."
    echo
    echo "*** UNIT TESTS FAILED ***"
    echo
    exit 1
  fi

done

# Success if we reach here
echo
echo "*** ALL UNIT TESTS PASSED ***"
echo
