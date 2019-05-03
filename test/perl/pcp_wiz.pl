#!/usr/bin/perl

use lib "/home/pgoldenb/src/plMetUtil/lib";

use strict;
use warnings;

use POSIX;
use VxUtil;
use XML::Parser;

my $init = "20050807_00";

# start the logging mechanism
my $log_file = "";
vx_log_open($log_file, 1, 1, 0);
vx_log("unit - running at " . strftime("%Y-%m-%d %H:%M:%S", gmtime()) . "\n" .
       "   INIT = $init\n");

my @leads = vx_util_seq(vx_const_get("LEAD_SEQ"));

# build the list of GRIB files
my %tmpl = ( init => $init );
my @gribs;
$tmpl{lead} = $_ 
  and push @gribs, vx_file_tmpl_gen(vx_const_get("GRIB_TMPL"), \%tmpl)
  for @leads;

# remove the first two lead times
shift @leads for 1..2;

# build the list of valid times
my @valids;
push @valids, vx_date_calc_add($init, $_) for @leads;

# build the map of file accumulations
my %pcp_map = %{ vx_pcp_build_map(@gribs) };
#vx_pcp_print_map( \%pcp_map );

# call the pcp_wizard
my $proc = vx_pcp_wizard_sub(
  6,                        # accum
  \@valids,                 # valid
  \%pcp_map,                # pcp_map
  vx_const_get("OUT_TMPL"), # out_tmpl
  {},                       # pcp_opts
  1,                        # verb
  -1,                       # accum_fix
  1                         # run_pcp
);

# close the log resource
vx_log("\nunit complete at " . strftime("%Y-%m-%d %H:%M:%S", gmtime()) . "\n");
vx_log_close();
