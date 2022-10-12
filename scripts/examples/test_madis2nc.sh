#!/bin/sh

echo
echo "*** Running MADIS2NC to reformat MADIS point observations into NetCDF ***"
madis2nc \
  ../data/sample_obs/madis/metar/metar_2012040912_F000.nc \
  ${TEST_OUT_DIR}/madis2nc/metar_2012040912_F000.nc \
  -type metar -rec_beg 2000 -rec_end 6000 -v 2
