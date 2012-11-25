#!/usr/bin/env perl


use strict;

open(ALL_EVENTS,"<patterns_out");
my $T = 0;
while(<ALL_EVENTS>) {
    my @line = split(',',$_);
    $T += @line;
}
close(ALL_EVENTS);
# T = 12559936
print $T . "\n";
