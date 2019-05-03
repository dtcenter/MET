#!/usr/bin/perl

use strict;
use warnings;

# check the return status and output files
my $mgnc = "/home/pgoldenb/src/rMnc/mgnc.sh";
my $test = {
  "out_nc"   => [
#    "/home/pgoldenb/src/rMgnc/data/test_bad.nc",
#    "/home/pgoldenb/src/rMgnc/data/test_missing.nc",
#    "/home/pgoldenb/src/rMgnc/data/test_zero.nc",
    "/home/pgoldenb/src/rMgnc/data/test_valids.nc"
  ],
  "out_stat" => [
#    "/home/pgoldenb/src/rMgnc/data/test_hdronly.stat",
#    "/home/pgoldenb/src/rMgnc/data/test_empty.stat",
    "/home/pgoldenb/src/rMgnc/data/test_valid_cnt.txt",
    "/home/pgoldenb/src/rMgnc/data/test_valid_nohdr.stat",
    "/home/pgoldenb/src/rMgnc/data/test_valid.stat"
  ]
};

my $out_ok = 1;
for my $output ( @{$test->{"out_nc"}} ){
  -s $output 
    or $out_ok = 0 && last;
  qx/$mgnc $output/;
  $? and $out_ok = 0;
  printf "out_nc  : %-55s: %s\n", $output, ( $? ? "fail" : "pass" );
}
print "out_nc  : " . ($out_ok ? "pass" : "fail") . "\n\n";

for my $output ( @{$test->{"out_stat"}} ){
  my $cmd_stat = "cat $output 2>/dev/null | egrep -v '^VERSION' | wc -l";
  my $num_lines = qx/$cmd_stat/;
  1 > $num_lines and $out_ok = 0;
  printf "out_stat: %-55s: %s\n", $output, ( 1 > $num_lines ? "fail" : "pass" );
}
print "out_stat: " . ($out_ok ? "pass" : "fail") . "\n";

