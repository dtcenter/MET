#!/bin/sh

echo
echo "*** Running ASCII2NC to reformat ASCII point observations into NetCDF ***"
../bin/ascii2nc \
   ../data/sample_obs/ascii/sample_ascii_obs.txt \
   ../out/ascii2nc/sample_ascii.nc \
   -v 2
