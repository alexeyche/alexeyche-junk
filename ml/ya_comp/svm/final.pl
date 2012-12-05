#!/usr/bin/env perl

use strict;
use autodie;
use Data::Dumper;


my @files = grep { -f } glob( './prediction/*_answer' );
my @fhs = map { open my $fh, '<', $_; $fh } @files;


my %model_to_freq;
open(FREQ, "<episode_out.freq.sort");
while(<FREQ>) {
    chomp($_);
    my @line = split(/\t/,$_);
    $model_to_freq{$line[0]} = $line[1];
}


for(my $i=0; $i<@files; $i++) {
    my @filename_line = split(/\//, $files[$i]);
    my $model = $filename_line[-1];
    $model =~ s/_answer$//;
    my $cur_fh = $fhs[$i];
    while(<$cur_fh>) {
        chomp($_);
        my @line = split(/ /, $_);
        my $rank = $line[-1] * $model_to_freq{$model};
        if(($model eq "others" ) && ($rank == 0 )) {
            $rank = $line[-1];
        }
        if($model eq 'Q') {
            $rank =  $model_to_freq{$model};
        }
        print $line[0] ." " .$rank . " " . $model ."\n";    
    }
}

