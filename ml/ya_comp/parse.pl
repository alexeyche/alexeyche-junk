#!/usr/bin/env perl

use strict;
use Data::Dumper;
use List::Util qw(sum);

open(TRAIN,'<dataset/train');
open(OUT, '>parse_out');

my $first_time=1;

my $train_set_l=52001965;
my $lines_num = 0;

# variables of common use
#my @pattern; 
my @click_query_pos;
my $click_count = 0;
my $aband_rate = 0;
my $current_serp_id;
my %serp;

while(<TRAIN>) {
    chomp($_);
    my $line = $_;
    my @line = split('\t',$_,6);
    my $sess_id = $line[0];
    my $sess_type = $line[2];
# percentage
    $lines_num = $lines_num+1; 
    if ($lines_num % 5000000 == 0) {
        print $lines_num/$train_set_l . "\n";
    }
# ------    
    if ($sess_type eq "M") {
        if ($first_time) {
            $first_time = 0;
        } else {
            # write stats
#            my $patt_j = join('',@pattern);
#            print PATTERNS $patt_j . "\n";
#            undef(@pattern);
             my $AvgClickPos = sum(@click_query_pos)/@click_query_pos;           
             print OUT $AvgClickPos ."\t". $click_count ."\t".$aband_rate ."\n";
             
             # clean for yourself!!!
             foreach my $k (keys %serp) {
                undef($serp{$k});
             }
             undef(%serp);
             $click_count = 0;
             $aband_rate = 0;
        }
        next;
    }
    #counts stats
#    if (($sess_type eq "Q") or ($sess_type eq "C")) {
#        push(@pattern,$sess_type);
#    }
    if ($sess_type eq "Q") {
        my @urls = split(/\t/,$line[5]);     
        my $serp_id = $line[3];
        $current_serp_id = $serp_id;
        my %current_query;
        @current_query{@urls} = (1 .. ($#urls+1));
        $serp{$serp_id} = \%current_query;
#        print Dumper(\%serp);
#        close(TRAIN);
#        exit(0);
    }
    if ($sess_type eq "C") {
        $click_count++;
        my $click_query = $line[4];
        my $serp_id = $line[3];
        if($serp_id != $current_serp_id) {
            $aband_rate ++;
        }
        my %current_query = %{ $serp{$serp_id} };
        if(exists $current_query{$click_query}) {
            push @click_query_pos, $current_query{$click_query};   
        } else {
            print "Click on query which not presented in session, ID $sess_id\n";
            exit(1);
        }
    }

}

close(TRAIN);
#close(PATTERNS);
exit(0);
