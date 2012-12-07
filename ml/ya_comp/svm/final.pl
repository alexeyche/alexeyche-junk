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
    my $model = $line[0];
    $model =~ s/,/_/g;   
    $model_to_freq{$model} = $line[1];
}

my $avg_others_freq = 10.30835;

for(my $i=0; $i<@files; $i++) {
    my @filename_line = split(/\//, $files[$i]);
    my $model = $filename_line[-1];
    $model =~ s/_answer$//;
    my $cur_fh = $fhs[$i];
    while(<$cur_fh>) {
        chomp($_);
        my @line = split(/ /, $_);
        my $freq = $model_to_freq{$model};
        if($model eq "others" ) {
            $freq = $avg_others_freq;
        } 
        my $rank = $line[-1] * $freq;
        
        if($model eq 'Q') {
            $rank =  $model_to_freq{$model};
        }
        print $line[0] ." " .$rank . " " . $model ." " .$line[-1] ."\n";    
    }
}

