#!/bin/sh

echo
echo "*** Running STAT-Analysis ***"
../bin/stat_analysis \
   -config config/STATAnalysisConfig \
   -lookin ../out/grid_stat \
   -lookin ../out/wavelet_stat \
   -out ../out/stat_analysis/stat_analysis.out \
   -v 2
