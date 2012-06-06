#!/usr/bin/perl

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#
# mode_conv.pl - a utility to convert MODE text output files
# to the STAT format consistent with the output of other MET
# tools
#
# last revised: 2012-01-18
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

use warnings;
use strict;

sub usage(){
  return "usage: $0 [-s|-p] {mode_file}\n" .
         "  The lines of the input MODE output file are reformatted as a MET\n" .
         "  stat file.  Depending on whether or not the input file is an _obj or\n" .
         "  _cts file, and the presence of the -s or -p parameter, only lines of\n" .
         "  the specified type are written, along with column headers.\n\n" .
         "  where:\n" .
         "    -s writes out only single object attributes with column headers\n" .
         "    -p writes out only pair object attributes with column headers\n";
}

my @fld_hdrs  = qw(VERSION MODEL FCST_LEAD FCST_VALID_BEG FCST_VALID_END OBS_LEAD OBS_VALID_BEG 
                   OBS_VALID_END FCST_VAR FCST_LEV OBS_VAR OBS_LEV OBTYPE VX_MASK INTERP_MTHD 
                   INTERP_PNTS FCST_THRESH OBS_THRESH COV_THRESH ALPHA LINE_TYPE);
                  
my @fld_sings = qw(OBJECT_ID OBJECT_CAT CENTROID_X CENTROID_Y CENTROID_LAT CENTROID_LON 
                   AXIS_ANG LENGTH WIDTH AREA AREA_FILTER AREA_THRESH CURVATURE CURVATURE_X 
                   CURVATURE_Y COMPLEXITY INTENSITY_10 INTENSITY_25 INTENSITY_50 INTENSITY_75 
                   INTENSITY_90 INTENSITY_50 INTENSITY_SUM);

my @fld_pairs = qw(OBJECT_ID OBJECT_CAT CENTROID_DIST BOUNDARY_DIST CONVEX_HULL_DIST ANGLE_DIFF 
                   AREA_RATIO INTERSECTION_AREA UNION_AREA SYMMETRIC_DIFF INTERSECTION_OVER_AREA 
                   COMPLEXITY_RATIO PERCENTILE_INTENSITY_RATIO INTEREST);

my @fld_ctss  = qw(FIELD TOTAL FY_OY FY_ON FN_OY FN_ON BASER FMEAN ACC FBIAS PODY PODN POFD FAR 
                   CSI GSS HK HSS ODDS);

my $fmt_hdr = 
      "%-8s"  . # VERSION
      "%-12s" . # MODEL
      "%-10s" . # FCST_LEAD
      "%-16s" . # FCST_VALID_BEG
      "%-16s" . # FCST_VALID_END
      "%-9s"  . # OBS_LEAD
      "%-16s" . # OBS_VALID_BEG
      "%-16s" . # OBS_VALID_END
      "%-12s" . # FCST_VAR
      "%-9s"  . # FCST_LEV
      "%-12s" . # OBS_VAR
      "%-9s"  . # OBS_LEV
      "%-10s" . # OBTYPE
      "%-8s"  . # VX_MASK
      "%-12s" . # INTERP_MTHD
      "%-12s" . # INTERP_PNTS
      "%-12s" . # FCST_THRESH
      "%-11s" . # OBS_THRESH
      "%-11s" . # COV_THRESH
      "%-6s"  . # ALPHA
      "%-10s";  # LINE_TYPE

my $fmt_sing = 
      "%-12s" . # OBJECT_ID
      "%-12s" . # OBJECT_CAT
      "%11s"  . # CENTROID_X
      "%11s"  . # CENTROID_Y
      "%13s"  . # CENTROID_LAT
      "%13s"  . # CENTROID_LON
      "%12s"  . # AXIS_ANG
      "%12s"  . # LENGTH
      "%10s"  . # WIDTH
      "%6s"   . # AREA
      "%12s"  . # AREA_FILTER
      "%12s"  . # AREA_THRESH
      "%10s"  . # CURVATURE
      "%12s"  . # CURVATURE_X
      "%12s"  . # CURVATURE_Y
      "%11s"  . # COMPLEXITY
      "%13s"  . # INTENSITY_10
      "%13s"  . # INTENSITY_25
      "%13s"  . # INTENSITY_50
      "%13s"  . # INTENSITY_75
      "%13s"  . # INTENSITY_90
      "%13s"  . # INTENSITY_50
      "%14s";   # INTENSITY_SUM
       
my $fmt_pair = 
      "%-12s" . # OBJECT_ID
      "%-12s" . # OBJECT_CAT
      "%14s"  . # CENTROID_DIST
      "%14s"  . # BOUNDARY_DIST
      "%17s"  . # CONVEX_HULL_DIST
      "%11s"  . # ANGLE_DIFF
      "%11s"  . # AREA_RATIO
      "%18s"  . # INTERSECTION_AREA
      "%11s"  . # UNION_AREA
      "%15s"  . # SYMMETRIC_DIFF
      "%23s"  . # INTERSECTION_OVER_AREA
      "%17s"  . # COMPLEXITY_RATIO
      "%27s"  . # PERCENTILE_INTENSITY_RATIO
      "%12s";   # INTEREST

my $fmt_cts = 
      "%7s"   . # FIELD
      "%8s"   . # TOTAL
      "%8s"   . # FY_OY
      "%8s"   . # FY_ON
      "%8s"   . # FN_OY
      "%8s"   . # FN_ON
      "%10s"  . # BASER
      "%10s"  . # FMEAN
      "%10s"  . # ACC
      "%10s"  . # FBIAS
      "%10s"  . # PODY
      "%10s"  . # PODN
      "%10s"  . # POFD
      "%10s"  . # FAR
      "%10s"  . # CSI
      "%10s"  . # GSS
      "%10s"  . # HK
      "%10s"  . # HSS
      "%10s";   # ODDS


if( 1 > @ARGV && 2 < @ARGV ){ die "ERROR: unexpected number of arguments\n\n" . usage() }

my $type = "b";
my $file = pop @ARGV;

# parse the input arguments
if( $file =~ /_cts\.txt$/ ){ $type = "c"; }
if( @ARGV >= 1 ){
  if( $type ne "b" ){ die "ERROR: incompatible argument with cts file\n\n" . usage() }
  $type = shift @ARGV;
  unless( $type =~ s/^\-([sp])$/$1/ ){ die "ERROR: unrecognized argument $type\n\n" . usage() }
}

# open the input file
open(my $fh_mode_in, "<", $file) or die "ERROR: cannot open input file $file\n\n" . usage();

# print the column headers, if appropriate
if( $type ne "b" ){
  my $fmt_val = "";
  my @fld_vals;

  if   ( $type eq "s" ){ $fmt_val = $fmt_sing; @fld_vals = @fld_sings; }
  elsif( $type eq "p" ){ $fmt_val = $fmt_pair; @fld_vals = @fld_pairs; }
  elsif( $type eq "c" ){ $fmt_val = $fmt_cts;  @fld_vals = @fld_ctss;  }

  printf( "${fmt_hdr}${fmt_val}\n", (@fld_hdrs, @fld_vals) );
}

# handle each input file line
while(<$fh_mode_in>){
  chomp();

  # print out the header line
  next if( /^VERSION/ );

  # parse the data line, and build the header
  my @vals = split /\s+/;
  my @outs = (@vals[0,1,2,3,3,5,6,6,12,13,14,15],
    "ANALYS",      # OBTYPE
    "FULL",        # VX_MASK
    "UW_MEAN",     # INTERP_MTHD
    "0",           # INTERP_PNTS
    $vals[9],      # FCST_THRESH
    $vals[11],     # OBS_THRESH
    $vals[8],      # COV_THRESH (FCST_RAD)
    $vals[10]      # ALPHA (OBS_RAD)
  );

  # write a cts line
  my $fmt_val;
  if( $type eq "c" ){
    push @outs, ("MODE_CTS", @vals[16 .. 34]);
    $fmt_val = $fmt_cts;
  } 
  
  # write a pair object attribute line 
  elsif( $vals[16] !~ /_/ ){
    next if( $type eq "p");
    push @outs, ("MODE_SOA", @vals[16 .. 40]);
    $fmt_val = $fmt_sing;
  } 
  
  # write a single object attribute line
  else {
    next if ($type eq "s");
    push @outs, ("MODE_POA", @vals[16,17,39 .. 50]);
    $fmt_val = $fmt_pair;
  } 

  # print the line
  printf("${fmt_hdr}${fmt_val}\n", @outs);
}

close($fh_mode_in);


#  0 - VERSION
#  1 - MODEL
#  2 - FCST_LEAD
#  3 - FCST_VALID
#  4 - FCST_ACCUM
#  5 - OBS_LEAD
#  6 - OBS_VALID
#  7 - OBS_ACCUM
#  8 - FCST_RAD
#  9 - FCST_THR
# 10 - OBS_RAD
# 11 - OBS_THR
# 12 - FCST_VAR
# 13 - FCST_LEV
# 14 - OBS_VAR
# 15 - OBS_LEV

# 16 - OBJECT_ID
# 17 - OBJECT_CAT
# 18 - CENTROID_X
# 19 - CENTROID_Y
# 20 - CENTROID_LAT
# 21 - CENTROID_LON
# 22 - AXIS_ANG
# 23 - LENGTH
# 24 - WIDTH
# 25 - AREA
# 26 - AREA_FILTER
# 27 - AREA_THRESH
# 28 - CURVATURE
# 29 - CURVATURE_X
# 30 - CURVATURE_Y
# 31 - COMPLEXITY
# 32 - INTENSITY_10
# 33 - INTENSITY_25
# 34 - INTENSITY_50
# 35 - INTENSITY_75
# 36 - INTENSITY_90
# 37 - INTENSITY_50
# 38 - INTENSITY_SUM
# 39 - CENTROID_DIST
# 40 - BOUNDARY_DIST
# 41 - CONVEX_HULL_DIST
# 42 - ANGLE_DIFF
# 43 - AREA_RATIO
# 44 - INTERSECTION_AREA
# 45 - UNION_AREA
# 46 - SYMMETRIC_DIFF
# 47 - INTERSECTION_OVER_AREA
# 48 - COMPLEXITY_RATIO
# 49 - PERCENTILE_INTENSITY_RATIO
# 50 - INTEREST

# 16 - FIELD
# 17 - TOTAL
# 18 - FY_OY
# 19 - FY_ON
# 20 - FN_OY
# 21 - FN_ON
# 22 - BASER
# 23 - FMEAN
# 24 - ACC
# 25 - FBIAS
# 26 - PODY
# 27 - PODN
# 28 - POFD
# 29 - FAR
# 30 - CSI
# 31 - GSS
# 32 - HK
# 33 - HSS
# 34 - ODDS


