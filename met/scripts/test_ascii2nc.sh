#!/bin/sh

echo
echo "*** Running ASCII2NC to reformat ASCII point observations into NetCDF ***"
../bin/ascii2nc \
   ../data/sample_obs/ascii/sample_ascii_obs.txt \
   ../out/ascii2nc/sample_ascii.nc \
   -v 2

echo
echo "*** Running ASCII2NC to reformat ASCII rain gauge observations into NetCDF ***"
../bin/ascii2nc \
   ../data/sample_obs/ascii/precip24_2010010112.ascii \
   ../out/ascii2nc/precip24_2010010112.nc \
   -v 2
