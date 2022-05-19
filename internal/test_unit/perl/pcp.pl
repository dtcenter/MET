#!/usr/bin/perl

use lib "/home/pgoldenb/src/plMetUtil/lib";
use VxUtil;

use strict;
use warnings;

my $met_base  = "/d1/pgoldenb/opt/MET_builds/svn-met-dev.cgd.ucar.edu/trunk/met";
my $grib_dir  = "/d1/pgoldenb/var/data/model_data/grib2/nam";
my $nc_dir    = "/d1/pgoldenb/var/data/model_data/met_nc/nam";
my $date_init = "20120409_00";

# generate 24hr precip using sum
for my $lead (vx_util_seq(24, 12, 84)){
  
  print "\n# # # #  LEAD $lead\n\n";
  
  my $out_file = $nc_dir . sprintf("/nam_2012040900_F0%02d_APCP24.nc", $lead);
  my $date_vld = vx_date_calc_add($date_init, $lead);    
  my $cmd = "$met_base/bin/pcp_combine -sum $date_init 12 $date_vld 24 \\\n" .
            "  $out_file \\\n  -pcpdir $grib_dir";
  print "PCP_COMBINE: $cmd\n";
  my @outs = qx/$cmd 2>&1/;
  print for @outs;
  print "\n";

}

# generate 3hr and 12hr precip using a -add pass-through
#for my $lead (vx_util_seq(3, 3, 84)){
#  
#  print "\n# # # #  LEAD $lead\n\n";
#  my $in_file  = $grib_dir . sprintf("/nam_2012040900_F0%02d.grib2", $lead);
#  
#  for my $acc (3, 12){
#    $acc == 12 and $lead % 12 and next;
#    
#    my $out_file = $nc_dir . sprintf("/nam_2012040900_F0%02d_APCP%02d.nc", $lead, $acc);    
#    my $cmd = "$met_base/bin/pcp_combine -add \\\n  $in_file $acc \\\n  $out_file";
#    print "PCP_COMBINE: $cmd\n";
#    my @outs = qx/$cmd 2>&1/;
#    print for @outs;
#    print "\n";
#  }
#
#}
