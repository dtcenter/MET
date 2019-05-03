#!/bin/sh

echo
echo "*** Running STAT-Analysis ***"
stat_analysis \
   -config config/STATAnalysisConfig \
   -lookin ${TEST_OUT_DIR}/grid_stat \
   -lookin ${TEST_OUT_DIR}/wavelet_stat \
   -lookin ${TEST_OUT_DIR}/ensemble_stat \
   -out ${TEST_OUT_DIR}/stat_analysis/stat_analysis.out \
   -v 2
