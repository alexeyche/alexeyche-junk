#!/usr/bin/env perl

use strict;
use Data::Dumper;
use List::Util qw(sum);
use POSIX;

open(HIST,'<parse_out.norm.hist');

my @hist;
while(<HIST>) {
    my @hist_line = split('\t',$_);
    push @hist, \@hist_line; 
}
close(HIST);

my $ncol = @{$hist[0]};

my @mins = @{$hist[-2]};
my @maxs = @{$hist[-1]};
delete($hist[-1]);
delete($hist[-2]);

my $prec = $#hist;

open(TEST,'<parse_out.test.norm');
my @dens;
while(<TEST>) {
    my @line = split('\t',$_);
    my $den = 1;
    for(my $i=0; $i<$ncol; $i++) {
        my $step = (abs($mins[$i])+abs($maxs[$i]))/$prec;
        my $cell = floor(abs($line[$i]/$step));
        my $cur_den = $hist[$cell][$i];
        $den += $cur_den;
    }
    push @dens, $den;
}
close(TEST);

my @dens_sorted = sort { $b <=> $a } @dens;
my %den_to_range;
@den_to_range{@dens_sorted} = (0 .. $#dens);

my $i=0;
open(TASK_TEST,'<dataset/test');
while(<TASK_TEST>) {
    my @line = split('\t',$_);
    if ($line[2] eq 'M') {
        print $line[0] ."\t". $den_to_range{$dens[$i]}. "\n";
        $i++;
    }    
}
close(TASK_TEST);
