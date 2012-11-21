#!/usr/bin/perl -w

use strict;

my @signals = qw(PostBeginJob);

foreach my $signal (@signals) {
  s&(->|\.)watch\Q$signal\E\b&$1s$signal\.watch&g;
  s&s\Q$signal\E_&s$signal\.invoke&g;
}
