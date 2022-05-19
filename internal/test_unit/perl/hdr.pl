#!/usr/bin/perl

$met_dir = "/d1/pgoldenb/opt/MET_builds/svn-met-dev.cgd.ucar.edu/trunk/met";
#$met_dir = "/d1/pgoldenb/opt/MET_builds/METv3.0_gnu4";
#$met_dir = "/d1/pgoldenb/opt/MET_builds/METv2.0_gnu4";
foreach (qx/find $met_dir\/out\/*stat* -name '*.txt'/){
  chomp();
  $h = qx/head -1 $_/;
  $h =~ s/\s+/ /g;
  s/.*_(\w+)\.txt$/$1/;
  $l{uc($_)} = $h;
}
printf("%-8s: %s\n", $_, $l{$_}) for sort keys %l;
