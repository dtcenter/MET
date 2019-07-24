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
                   OBS_VALID_END FCST_VAR FCST_UNITS FCST_LEV OBS_VAR OBS_UNITS OBS_LEV OBTYPE VX_MASK
                   INTERP_MTHD INTERP_PNTS FCST_THRESH OBS_THRESH COV_THRESH ALPHA LINE_TYPE);

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
      "%-12s" . # FCST_UNITS
      "%-9s"  . # FCST_LEV
      "%-12s" . # OBS_VAR
      "%-12s" . # OBS_UNITS
      "%-9s"  . # OBS_LEV
      "%-10s" . # OBTYPE
      "%-8s"  . # VX_MASK
      "%-12s" . # INTERP_MTHD
      "%-12s" . # INTERP_PNTS
      "%-20s" . # FCST_THRESH
      "%-20s" . # OBS_THRESH
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
      "%12s"  . # BASER
      "%12s"  . # FMEAN
      "%12s"  . # ACC
      "%12s"  . # FBIAS
      "%12s"  . # PODY
      "%12s"  . # PODN
      "%12s"  . # POFD
      "%12s"  . # FAR
      "%12s"  . # CSI
      "%12s"  . # GSS
      "%12s"  . # HK
      "%12s"  . # HSS
      "%12s";   # ODDS


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
    $vals[16], # FCST_UNITS
    $vals[17], # FCST_LEV
    $vals[18], # OBS_VAR
    $vals[19], # OBS_UNITS
    $vals[20], # OBS_LEV
    $vals[21], # OBTYPE
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
    push @outs, (" MODE_CTS ", @vals[2,3,22 .. 40]);
    $fmt_val = $fmt_cts;
  }

  # write a single object attribute line
  elsif( $vals[20] !~ /_/ ){
    next if( $type eq "p");
    push @outs, (" MODE_SOA ", @vals[2,3,22 .. 43]);
    $fmt_val = $fmt_sing;
  }

  # write a pair object attribute line
  else {
    next if ($type eq "s");
    push @outs, (" MODE_POA ", @vals[2,3,22,23,44 .. 57]);
    $fmt_val = $fmt_pair;
  }

  # print the line
  printf("${fmt_hdr}${fmt_val}\n", @outs);
}

close($fh_mode_in);

#  0 - VERSION
#  1 - MODEL
#  2 - N_VALID
#  3 - GRID_RES
#  4 - DESC
#  5 - FCST_LEAD
#  6 - FCST_VALID
#  7 - FCST_ACCUM
#  8 - OBS_LEAD
#  9 - OBS_VALID
# 10 - OBS_ACCUM
# 11 - FCST_RAD
# 12 - FCST_THR
# 13 - OBS_RAD
# 14 - OBS_THR
# 15 - FCST_VAR
# 16 - FCST_UNITS
# 17 - FCST_LEV
# 18 - OBS_VAR
# 19 - OBS_UNITS
# 20 - OBS_LEV
# 21 - OBTYPE

# 22 - OBJECT_ID
# 23 - OBJECT_CAT

# 24 - CENTROID_X
# 25 - CENTROID_Y
# 26 - CENTROID_LAT
# 27 - CENTROID_LON
# 28 - AXIS_ANG
# 29 - LENGTH
# 30 - WIDTH
# 31 - AREA
# 32 - AREA_THRESH
# 33 - CURVATURE
# 34 - CURVATURE_X
# 35 - CURVATURE_Y
# 36 - COMPLEXITY
# 37 - INTENSITY_10
# 38 - INTENSITY_25
# 39 - INTENSITY_50
# 40 - INTENSITY_75
# 41 - INTENSITY_90
# 42 - INTENSITY_50
# 43 - INTENSITY_SUM

# 44 - CENTROID_DIST
# 45 - BOUNDARY_DIST
# 46 - CONVEX_HULL_DIST
# 47 - ANGLE_DIFF
# 48 - ASPECT_DIFF
# 49 - AREA_RATIO
# 50 - INTERSECTION_AREA
# 51 - UNION_AREA
# 52 - SYMMETRIC_DIFF
# 53 - INTERSECTION_OVER_AREA
# 54 - CURVATURE_RATIO
# 55 - COMPLEXITY_RATIO
# 56 - PERCENTILE_INTENSITY_RATIO
# 57 - INTEREST

# 22 - FIELD
# 23 - TOTAL
# 24 - FY_OY
# 25 - FY_ON
# 26 - FN_OY
# 27 - FN_ON
# 28 - BASER
# 29 - FMEAN
# 30 - ACC
# 31 - FBIAS
# 32 - PODY
# 33 - PODN
# 34 - POFD
# 35 - FAR
# 36 - CSI
# 37 - GSS
# 38 - HK
# 39 - HSS
# 40 - ODDS
