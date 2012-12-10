#!/usr/bin/env perl

use strict;

my $file = $ARGV[0];
open(INPUT,"<$file");


my %id_to_dens;

while(<INPUT>) {
    chomp;
    my @line = split(/\t/,$_);
    $id_to_dens{$line[0]} = sprintf("%f",$line[1]);
#    print $id_to_dens{$line[0]} . "\n";
}

close(INPUT);

sub hashValueAscendingNum {
       $id_to_dens{$a} <=> $id_to_dens{$b};
}

sub hashValueDescendingNum {
       $id_to_dens{$b} <=> $id_to_dens{$a};
}

foreach my $key (sort hashValueDescendingNum (keys(%id_to_dens))) {
   print "$key\t$id_to_dens{$key}\n";
}
