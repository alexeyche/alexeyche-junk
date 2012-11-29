#!/usr/bin/env perl

use strict;
use Data::Dumper;
use List::Util qw(sum);
use POSIX;

open(TEST,'<answer');
my @dens;
while(<TEST>) {
    my @line = split('\t',$_);
    my $den = $line[1]; 
    push @dens, $den;
}
close(TEST);

my @dens_sorted = sort { $b <=> $a } @dens;
my %den_to_range;
@den_to_range{@dens_sorted} = (0 .. $#dens);


my $i=0;
open(TASK_TEST,'<../dataset/test');
while(<TASK_TEST>) {
    my @line = split('\t',$_);
    if ($line[2] eq 'M') {
        print $line[0] ."\t". $den_to_range{$dens[$i]}. "\n";
        $i++;
    }    
}
close(TASK_TEST);
