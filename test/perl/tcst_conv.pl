#!/usr/bin/perl

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#
# tcst_conv.pl - a utility to convert TCST text output files
# to the STAT format consistent with the output of other MET
# tools
#
# last revised: 2016-09-27 for MET version 6.0
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

use warnings;
use strict;

sub usage(){
  return "usage: $0 {tcst_file}\n" .
         "  The lines of the input TCST output file are reformatted as a MET\n" .
         "  stat file.\n\n";
}

my @fld_hdrs  = qw(VERSION MODEL FCST_LEAD FCST_VALID_BEG FCST_VALID_END OBS_LEAD OBS_VALID_BEG 
                   OBS_VALID_END FCST_VAR FCST_LEV OBS_VAR OBS_LEV OBTYPE VX_MASK INTERP_MTHD 
                   INTERP_PNTS FCST_THRESH OBS_THRESH COV_THRESH ALPHA LINE_TYPE);

my @fld_tcst  = qw(AMODEL BMODEL STORM_ID BASIN CYCLONE STORM_NAME INIT_MASK VALID_MASK
                   TOTAL INDEX LEVEL WATCH_WARN INITIALS ALAT ALON BLAT BLON
                   TK_ERR X_ERR Y_ERR ALTK_ERR CRTK_ERR ADLAND BDLAND AMSLP BMSLP AMAX_WIND BMAX_WIND
                   AAL_WIND_34 BAL_WIND_34 ANE_WIND_34 BNE_WIND_34 ASE_WIND_34 BSE_WIND_34 ASW_WIND_34 BSW_WIND_34 ANW_WIND_34 BNW_WIND_34
                   AAL_WIND_50 BAL_WIND_50 ANE_WIND_50 BNE_WIND_50 ASE_WIND_50 BSE_WIND_50 ASW_WIND_50 BSW_WIND_50 ANW_WIND_50 BNW_WIND_50
                   AAL_WIND_64 BAL_WIND_64 ANE_WIND_64 BNE_WIND_64 ASE_WIND_64 BSE_WIND_64 ASW_WIND_64 BSW_WIND_64 ANW_WIND_64 BNW_WIND_64
                   ARADP BRADP ARRP BRRP AMRD BMRD AGUSTS BGUSTS AEYE BEYE ADIR BDIR ASPEED BSPEED ADEPTH BDEPTH);

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

my $fmt_tcst =
      "%15s"  . # AMODEL
      "%15s"  . # BMODEL
      "%15s"  . # STORM_ID
      "%15s"  . # BASIN
      "%15s"  . # CYCLONE
      "%15s"  . # STORM_NAME
      "%15s"  . # INIT_MASK
      "%15s"  . # VALID_MASK
      "%15s"  . # TOTAL
      "%15s"  . # INDEX
      "%15s " . # LEVEL
      "%15s"  . # WATCH_WARN
      "%15s"  . # INITIALS
      "%15s"  . # ALAT
      "%15s"  . # ALON
      "%15s"  . # BLAT
      "%15s"  . # BLON
      "%15s"  . # TK_ERR
      "%15s"  . # X_ERR
      "%15s"  . # Y_ERR
      "%15s"  . # ALTK_ERR
      "%15s"  . # CRTK_ERR
      "%15s"  . # ADLAND
      "%15s"  . # BDLAND
      "%15s"  . # AMSLP
      "%15s"  . # BMSLP
      "%15s"  . # AMAX_WIND
      "%15s"  . # BMAX_WIND
      "%15s"  . # AAL_WIND_34
      "%15s"  . # BAL_WIND_34
      "%15s"  . # ANE_WIND_34
      "%15s"  . # BNE_WIND_34
      "%15s"  . # ASE_WIND_34
      "%15s"  . # BSE_WIND_34
      "%15s"  . # ASW_WIND_34
      "%15s"  . # BSW_WIND_34
      "%15s"  . # ANW_WIND_34
      "%15s"  . # BNW_WIND_34
      "%15s"  . # AAL_WIND_50
      "%15s"  . # BAL_WIND_50
      "%15s"  . # ANE_WIND_50
      "%15s"  . # BNE_WIND_50
      "%15s"  . # ASE_WIND_50
      "%15s"  . # BSE_WIND_50
      "%15s"  . # ASW_WIND_50
      "%15s"  . # BSW_WIND_50
      "%15s"  . # ANW_WIND_50
      "%15s"  . # BNW_WIND_50
      "%15s"  . # AAL_WIND_64
      "%15s"  . # BAL_WIND_64
      "%15s"  . # ANE_WIND_64
      "%15s"  . # BNE_WIND_64
      "%15s"  . # ASE_WIND_64
      "%15s"  . # BSE_WIND_64
      "%15s"  . # ASW_WIND_64
      "%15s"  . # BSW_WIND_64
      "%15s"  . # ANW_WIND_64
      "%15s"  . # BNW_WIND_64
      "%15s"  . # ARADP
      "%15s"  . # BRADP
      "%15s"  . # ARRP
      "%15s"  . # BRRP
      "%15s"  . # AMRD
      "%15s"  . # BMRD
      "%15s"  . # AGUSTS
      "%15s"  . # BGUSTS
      "%15s"  . # AEYE
      "%15s"  . # BEYE
      "%15s"  . # ADIR
      "%15s"  . # BDIR
      "%15s"  . # ASPEED
      "%15s"  . # BSPEED
      "%15s"  . # ADEPTH
      "%15s";   # BDEPTH


if( 1 > @ARGV && 2 < @ARGV ){ die "ERROR: unexpected number of arguments\n\n" . usage() }

my $file = pop @ARGV;

# open the input file
open(my $fh_tcst_in, "<", $file) or die "ERROR: cannot open input file $file\n\n" . usage();

# print the column headers
printf( "${fmt_hdr}${fmt_tcst}\n", (@fld_hdrs, @fld_tcst) );

# handle each input file line
while(<$fh_tcst_in>){
  chomp();

  # print out the header line
  next if( /^VERSION/ );

  # parse the data line, and build the header
  my @vals = split /\s+/;
  my @outs = (
     $vals[0], # VERSION
     "NA",     # MODEL
     "NA",     # DESC 
     $vals[8], # FCST_LEAD
     $vals[9], # FCST_VALID_BEG
     $vals[9], # FCST_VALID_END
     "NA",     # OBS_LEAD
     $vals[9], # OBS_VALID_BEG
     $vals[9], # OBS_VALID_END
     "NA",     # FCST_VAR
     "NA",     # FCST_LEV
     "NA",     # OBS_VAR
     "NA",     # OBS_LEV
     "NA",     # OBTYPE
     "NA",     # VX_MASK
     "NA",     # INTERP_MTHD
     "NA",     # INTERP_PNTS
     "NA",     # FCST_THRESH
     "NA",     # OBS_THRESH
     "NA",     # COV_THRESH
     "NA",     # ALPHA
     "TCST"    # LINE_TYPE
  );

  # write the output line
  push @outs, (@vals[1 .. 6,10,11,13 .. 78]);

  # print the line
  printf("${fmt_hdr}${fmt_tcst}\n", @outs);
}

close($fh_tcst_in);

# TCST Header Line
#     0 VERSION
#     1 AMODEL
#     2 BMODEL
#     3 STORM_ID
#     4 BASIN
#     5 CYCLONE
#     6 STORM_NAME
#     7 INIT
#     8 LEAD
#     9 VALID
#    10 INIT_MASK
#    11 VALID_MASK
#    12 LINE_TYPE
#    13 TOTAL
#    14 INDEX
#    15 LEVEL
#    16 WATCH_WARN
#    17 INITIALS
#    18 ALAT
#    19 ALON
#    20 BLAT
#    21 BLON
#    22 TK_ERR
#    23 X_ERR
#    24 Y_ERR
#    25 ALTK_ERR
#    26 CRTK_ERR
#    27 ADLAND
#    28 BDLAND
#    29 AMSLP
#    30 BMSLP
#    31 AMAX_WIND
#    32 BMAX_WIND
#    33 AAL_WIND_34
#    34 BAL_WIND_34
#    35 ANE_WIND_34
#    36 BNE_WIND_34
#    37 ASE_WIND_34
#    38 BSE_WIND_34
#    39 ASW_WIND_34
#    40 BSW_WIND_34
#    41 ANW_WIND_34
#    42 BNW_WIND_34
#    43 AAL_WIND_50
#    44 BAL_WIND_50
#    45 ANE_WIND_50
#    46 BNE_WIND_50
#    47 ASE_WIND_50
#    48 BSE_WIND_50
#    49 ASW_WIND_50
#    50 BSW_WIND_50
#    51 ANW_WIND_50
#    52 BNW_WIND_50
#    53 AAL_WIND_64
#    54 BAL_WIND_64
#    55 ANE_WIND_64
#    56 BNE_WIND_64
#    57 ASE_WIND_64
#    58 BSE_WIND_64
#    59 ASW_WIND_64
#    60 BSW_WIND_64
#    61 ANW_WIND_64
#    62 BNW_WIND_64
#    63  ARADP
#    64  BRADP
#    65  ARRP
#    66  BRRP
#    67  AMRD
#    68  BMRD
#    69  AGUSTS
#    70  BGUSTS
#    71  AEYE
#    72  BEYE
#    73  ADIR
#    74  BDIR
#    75  ASPEED
#    76  BSPEED
#    77  ADEPTH
#    78  BDEPTH
