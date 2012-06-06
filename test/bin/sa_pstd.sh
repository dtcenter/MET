#!/bin/bash

#MET_BASE=/d1/pgoldenb/opt/MET_builds/METv3.0.1_gnu4
MET_BASE=/d1/pgoldenb/opt/MET_builds/svn-met-dev.cgd.ucar.edu/trunk/met

#HMT_DATA_DIR=/dakota_d3/projects/HMT/West/2011/dwr_domains
HMT_DATA_DIR=/d1/pgoldenb/var/hmt/2011/dwr_domains
DUMP_ROW_FILE=./sa_out/sa_pstd_dump.stat
OUT_FILE=./sa_out/sa_pstd_agg.out
  
$MET_BASE/bin/stat_analysis \
  -lookin $HMT_DATA_DIR/hmt-ens-d01/201012*/grid_stat \
  -v 2 \
  -fcst_lead "060000" \
  -fcst_init_hour "120000" \
  -line_type "PCT" \
  -fcst_var "APCP_06_A6_ENS_FREQ_ge12.700" \
  -vx_mask "LAND_d01" \
  -fcst_valid_beg "20101218_000000" \
  -fcst_valid_end "20101222_000000" \
  -dump_row $DUMP_ROW_FILE \
  -out $OUT_FILE \
  -job aggregate_stat -out_line_type PSTD
