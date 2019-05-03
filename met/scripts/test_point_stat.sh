#!/bin/sh

echo
echo "*** Running Point-Stat on sample NAM data ***"
../bin/point_stat \
   ../data/sample_fcst/2007033000/nam.t00z.awip1236.tm00.20070330.grb \
   ../out/pb2nc/sample_pb.nc \
   config/PointStatConfig \
   -ncfile ../out/ascii2nc/sample_ascii.nc \
   -outdir ../out/point_stat -v 2
