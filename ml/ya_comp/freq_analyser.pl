#!/usr/bin/env perl

use strict;
use Data::Dumper;

open(PATTERNS, "<patterns");

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

open(PATTERN_STAT, ">pattern_stat");

foreach my $k (keys %patterns) {
    print PATTERN_STAT "$k\t$patterns{$k}\n";
}

close(PATTERN_STAT);    
