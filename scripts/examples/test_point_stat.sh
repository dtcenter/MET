#!/bin/sh

echo
echo "*** Running Point-Stat on sample NAM data ***"
point_stat \
   ../data/sample_fcst/2007033000/nam.t00z.awip1236.tm00.20070330.grb \
   ${TEST_OUT_DIR}/pb2nc/sample_pb.nc \
   config/PointStatConfig \
   -point_obs ${TEST_OUT_DIR}/ascii2nc/sample_ascii.nc \
   -outdir ${TEST_OUT_DIR}/point_stat -v 2 
