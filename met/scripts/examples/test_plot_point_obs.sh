#!/bin/sh

echo
echo "*** Running PLOT_POINT_OBS on a sample netCDF observation file ***"
plot_point_obs \
   ${TEST_OUT_DIR}/pb2nc/sample_pb.nc \
   ${TEST_OUT_DIR}/plot_point_obs/sample_pb.ps
