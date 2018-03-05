#!/usr/bin/perl

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#
# mode_conv.pl - a utility to convert MODE text output files
# to the STAT format consistent with the output of other MET
# tools
#
# last revised: 2016-09-17 for MET version 6.0
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

my @fld_hdrs  = qw(VERSION MODEL DESC FCST_LEAD FCST_VALID_BEG FCST_VALID_END OBS_LEAD OBS_VALID_BEG
                   OBS_VALID_END FCST_VAR FCST_LEV OBS_VAR OBS_LEV OBTYPE VX_MASK INTERP_MTHD
                   INTERP_PNTS FCST_THRESH OBS_THRESH COV_THRESH ALPHA LINE_TYPE);

my @fld_sings = qw(N_VALID GRID_RES OBJECT_ID OBJECT_CAT CENTROID_X CENTROID_Y CENTROID_LAT CENTROID_LON
                   AXIS_ANG LENGTH WIDTH AREA AREA_THRESH CURVATURE CURVATURE_X
                   CURVATURE_Y COMPLEXITY INTENSITY_10 INTENSITY_25 INTENSITY_50 INTENSITY_75
                   INTENSITY_90 INTENSITY_50 INTENSITY_SUM);

my @fld_pairs = qw(N_VALID GRID_RES OBJECT_ID OBJECT_CAT CENTROID_DIST BOUNDARY_DIST CONVEX_HULL_DIST ANGLE_DIFF
                   AREA_RATIO INTERSECTION_AREA UNION_AREA SYMMETRIC_DIFF INTERSECTION_OVER_AREA
                   COMPLEXITY_RATIO PERCENTILE_INTENSITY_RATIO INTEREST);

my @fld_ctss  = qw(N_VALID GRID_RES FIELD TOTAL FY_OY FY_ON FN_OY FN_ON BASER FMEAN ACC FBIAS PODY PODN POFD FAR
                   CSI GSS HK HSS ODDS);

my $fmt_hdr =
      "%-8s"  . # VERSION
      "%-12s" . # MODEL
      "%-12s" . # DESC
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
      "%-12s" . # N_VALID
      "%-12s" . # GRID_RES
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
      "%-12s" . # N_VALID
      "%-12s" . # GRID_RES
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
      "%-12s" . # N_VALID
      "%-12s" . # GRID_RES
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
  my @outs = (
    $vals[0],  # VERSION
    $vals[1],  # MODEL
    $vals[4],  # DESC
    $vals[5],  # FCST_LEAD
    $vals[6],  # FCST_VALID_BEG
    $vals[6],  # FCST_VALID_END
    $vals[8],  # OBS_LEAD
    $vals[9],  # OBS_VALID_END
    $vals[9],  # OBS_VALID_END
    $vals[15], # FCST_VAR
    $vals[16], # FCST_LEV
    $vals[17], # OBS_VAR
    $vals[18], # OBS_LEV
    $vals[19], # OBTYPE
    "FULL",    # VX_MASK
    "UW_MEAN", # INTERP_MTHD
    "0",       # INTERP_PNTS
    $vals[12], # FCST_THRESH
    $vals[14], # OBS_THRESH
    $vals[11], # COV_THRESH (FCST_RAD)
    $vals[13]  # ALPHA (OBS_RAD)
  );

  # write a cts line
  my $fmt_val;
  if( $type eq "c" ){
    push @outs, (" MODE_CTS ", @vals[2,3,19 .. 37]);
    $fmt_val = $fmt_cts;
  }

  # write a pair object attribute line
  elsif( $vals[17] !~ /_/ ){
    next if( $type eq "p");
    push @outs, (" MODE_SOA ", @vals[2,3,19 .. 43]);
    $fmt_val = $fmt_sing;
  }

  # write a single object attribute line
  else {
    next if ($type eq "s");
    push @outs, (" MODE_POA ", @vals[2,3,19,20,42 .. 53]);
    $fmt_val = $fmt_pair;
  }

  # print the line
  printf("${fmt_hdr}${fmt_val}\n", @outs);
}

close($fh_mode_in);


#  0 - VERSION
#  1 - MODEL
#  2 - N_VALID
#  3  - GRID_RES
...
#  2 - DESC
#  3 - FCST_LEAD
#  4 - FCST_VALID
#  5 - FCST_ACCUM
#  6 - OBS_LEAD
#  7 - OBS_VALID
#  8 - OBS_ACCUM
#  9 - FCST_RAD
# 10 - FCST_THR
# 11 - OBS_RAD
# 12 - OBS_THR
# 13 - FCST_VAR
# 14 - FCST_LEV
# 15 - OBS_VAR
# 16 - OBS_LEV

# 17 - OBJECT_ID
# 18 - OBJECT_CAT
# 19 - CENTROID_X
# 20 - CENTROID_Y
# 21 - CENTROID_LAT
# 22 - CENTROID_LON
# 23 - AXIS_ANG
# 24 - LENGTH
# 25 - WIDTH
# 26 - AREA
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

# 17 - FIELD
# 18 - TOTAL
# 19 - FY_OY
# 20 - FY_ON
# 21 - FN_OY
# 22 - FN_ON
# 23 - BASER
# 24 - FMEAN
# 25 - ACC
# 26 - FBIAS
# 27 - PODY
# 28 - PODN
# 29 - POFD
# 30 - FAR
# 31 - CSI
# 32 - GSS
# 33 - HK
# 34 - HSS
# 35 - ODDS


