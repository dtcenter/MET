#!/usr/bin/perl

use strict;
use warnings;

use Time::HiRes qw( usleep gettimeofday tv_interval );
use POSIX;

#my $t0 = [gettimeofday];
#usleep(126250000);
#my $elapsed = tv_interval ( $t0 );

my $elapsed = 126.2500;
printf "%8.3f", $elapsed;
