#!/usr/bin/env perl

use strict;
use Data::Dumper;

my $file = $ARGV[0];
open(PATTERNS, "<$file");

my %patterns;
while(<PATTERNS>) {
    chomp($_);
    my $pattern = $_;
    if(exists $patterns{$pattern}) {
        $patterns{$pattern} = $patterns{$pattern} + 1;    
    } else {
        $patterns{$pattern} = 1;
    }
}

close(PATTERNS); 

print "Patterns is counted\nWriting to file...\n";

open(PATTERNS_STAT, ">patterns_stat");

foreach my $k (keys %patterns) {
    print PATTERNS_STAT "$k\t$patterns{$k}\n";
}

close(PATTERNS_STAT);    
