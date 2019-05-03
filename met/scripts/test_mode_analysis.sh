#!/bin/sh

echo
echo "*** Running MODE-Analysis to compute column summaries for simple forecast objects ***"

../bin/mode_analysis \
   -lookin ../out/mode/mode_APCP_12_SFC_vs_APCP_12_SFC_120000L_20050807_120000V_120000A_obj.txt \
   -summary \
   -column AREA -column LENGTH -column WIDTH -column CURVATURE -column INTENSITY_50 \
   -fcst -config config/MODEAnalysisConfig \
   -dump_row ../out/mode_analysis/job_summary_APCP_12_simple_fcst.txt \
   -out      ../out/mode_analysis/job_summary_APCP_12_simple_fcst.out

echo
echo "*** Running MODE-Analysis to compute column summaries for simple observation objects ***"

../bin/mode_analysis \
   -lookin ../out/mode/mode_APCP_12_SFC_vs_APCP_12_SFC_120000L_20050807_120000V_120000A_obj.txt \
   -summary \
   -column AREA -column LENGTH -column WIDTH -column CURVATURE -column INTENSITY_50 \
   -obs -config config/MODEAnalysisConfig \
   -dump_row ../out/mode_analysis/job_summary_APCP_12_simple_obs.txt \
   -out      ../out/mode_analysis/job_summary_APCP_12_simple_obs.out

echo
echo "*** Running MODE-Analysis to summarize by case ***"

../bin/mode_analysis \
   -lookin ../out/mode/mode_APCP_12_SFC_vs_APCP_12_SFC_120000L_20050807_120000V_120000A_obj.txt \
   -lookin ../out/mode/mode_APCP_24_SFC_vs_APCP_24_SFC_240000L_20050808_000000V_240000A_obj.txt \
   -bycase -single -simple \
   -dump_row ../out/mode_analysis/job_summary_bycase.txt \
   -out      ../out/mode_analysis/job_summary_bycase.out
