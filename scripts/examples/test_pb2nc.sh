#!/bin/sh

echo
echo "*** Running PB2NC on a PREPBUFR file ***"
pb2nc \
   ../data/sample_obs/prepbufr/ndas.t00z.prepbufr.tm12.20070401.nr \
   ${TEST_OUT_DIR}/pb2nc/sample_pb.nc \
   config/PB2NCConfig_G212 \
   -v 2
