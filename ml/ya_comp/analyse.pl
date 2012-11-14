#!/usr/bin/env perl

use strict;

# initial data
my $uniq_q = 9415777;
my $q_file = "queries_freq.sort.top1000";
my $top = 1000;

open(TOPQ, "<$q_file");
open(OUT, ">q_class");
my $line_count = 0;
while(<TOPQ>) {
    my $class = "bad3";
    my $perc_l = $line_count/$top;
    if ($perc_l <= 0.02) {
        $class = "bad1";
    } elsif ($perc_l <= 0.5) {
        $class = "bad2"
    } 
    my @line = split(/\t/, $_);
    print OUT $line[0] . "\t" . $line[1]/$uniq_q . "\t" . $class . "\n";
    $line_count++;
}

close(TOPQ);
