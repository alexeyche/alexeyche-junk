#!/usr/bin/env perl

use strict;

open(TRAIN,'<dataset/train');
open(PATTERNS, '>patterns');

my $first_time=1;

my $train_set_l=52001965;
my $lines_num = 0;

my @pattern;

while(<TRAIN>) {
    chomp($_);
    my $line = $_;
    my @line = split('\t',$_,5);
    my $sess_id = $line[0];
    my $sess_type = $line[2];
    $lines_num = $lines_num+1; 
    if ($lines_num % 5000000 == 0) {
        print $lines_num/$train_set_l . "\n";
    }
    if ($sess_type eq "M") {
        if ($first_time) {
            $first_time = 0;
        } else {
            # write stats
            my $patt_j = join('',@pattern);
            print PATTERNS $patt_j . "\n";
            undef(@pattern);
        }
        next;
    }
    #counts stats
    if (($sess_type eq "Q") or ($sess_type eq "C")) {
        push(@pattern,$sess_type);
    }
}

close(TRAIN);
close(PATTERNS);
exit(0);
