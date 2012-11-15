#!/usr/bin/env perl

use strict;
use Data::Dumper;
use List::Util qw(sum);

sub uniq {
    return keys %{{ map { $_ => 1 } @_ }};
}

# load query classes
open(QCLASS, '<q_class');
my %query_class;
my %query_prob;
while(<QCLASS>) {
    my @line = split('\t',$_);
    $query_prob{$line[0]} = $line[1];    
    $query_class{$line[0]} = $line[2];    
}
close(QCLASS);

open(TRAIN,'<dataset/test');
open(OUT, '>parse_out.test');

my $first_time=1;

my $train_set_l=52001965;
my $lines_num = 0;

# features
my $AvgPosCount=0;
my $DwellTimeUntilClick=0;
my $SumDensBadQuery=0;
my $NumBackSerp=0;
# sevice vars
my @clicks_pos;
my @dwell_times_until_click;
my $last_was_query=0;
my $curr_serp;
my %serp;
my $click_count=0;

while(<TRAIN>) {
    chomp($_);
    my $line = $_;
    my @line = split('\t',$line,6);
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
            if ($click_count != 0) {
                $AvgPosCount = sum(@clicks_pos)/@clicks_pos;
                $DwellTimeUntilClick = sum(@dwell_times_until_click)/@dwell_times_until_click;
            } else {
                $AvgPosCount=-1;
                $DwellTimeUntilClick=-1;    
            }
            print OUT $AvgPosCount ."\t". $DwellTimeUntilClick ."\t". $SumDensBadQuery ."\t". $NumBackSerp . "\n";

            foreach my $k (keys %serp) {
                undef($serp{$k});
            }
            undef(%serp);
            undef(@clicks_pos);
            undef(@dwell_times_until_click);
            $last_was_query=0;
            $click_count=0;            
            
            $AvgPosCount=0;
            $DwellTimeUntilClick=0;
            $SumDensBadQuery=0;
            $NumBackSerp=0;
        }
        next;
    }
    if ($sess_type eq "Q") {
        $curr_serp = $line[3];
        my $query_id = $line[4];
        if (exists $query_prob{$query_id}) {
            $SumDensBadQuery += $query_prob{$query_id};
        }
        my %current_query;
        my @urls = split(/\t/,$line[5]);
        @current_query{@urls} = (1 .. ($#urls+1));
        $serp{$curr_serp} = \%current_query;
        
        #for DwellTimeUntilClick 
        $last_was_query=1;
        
        undef($query_id);
        undef(@urls);
    } else {
        $last_was_query=0;
    }
    if ($sess_type eq "C") {
        $click_count += 1;
        my $serp_id = $line[3];        
        my $query_id = $line[4];
        push @clicks_pos, $serp{$serp_id}{$query_id};
        if(not $curr_serp == $serp_id ) {
             $NumBackSerp += 1;
        }
        push @dwell_times_until_click, $line[1];
        undef($serp_id);
        undef($query_id);
    }
    undef($sess_id);
    undef($line);
    undef(@line);
    undef($sess_type);
}

close(TRAIN);
#close(PATTERNS);
exit(0);
