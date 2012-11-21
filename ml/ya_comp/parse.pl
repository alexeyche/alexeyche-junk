#!/usr/bin/env perl

use strict;
use Data::Dumper;
use List::Util qw(sum);
use POSIX;
require 'rate.pl';

sub uniq {
    return keys %{{ map { $_ => 1 } @_ }};
}

sub similarity {
    my ($arr1, $arr2) = @_;
    my @arr1 = @{$arr1};
    my %arr1 = map { $_ => 1 } @arr1;
    my $sim = 0;
    foreach my $el (@{$arr2}) {
        if(exists($arr1{$el})) {
            $sim += 1/$#arr1;
        }
    }
    return $sim;
}

sub make_out_s {
    my @arr = @_;
    my $ind = $#arr;
    my $str = "";
    for(my $i=0; $i <= $ind; $i++) {
        my $sym = "\t";
        if($i == $ind) {
            $sym = "\n"            
        }
        $str = $str . $arr[$i] . $sym;             
    }
    return $str;
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

my $train_file;
if($ARGV[0] eq "-t") {
    $train_file = "dataset/test";    
    open(TRAIN,"<$train_file");
    open(OUT, '>parse_out.2.test');
} else {
    $train_file = "dataset/train";    
    open(TRAIN,"<$train_file");
    open(OUT, '>parse_out.2');
}

my $first_time=1;

my $train_set_l=`./proc count_rows $train_file`;
my $ten_portion = floor($train_set_l/10);
my $lines_num = 0;

# features
my $AvgPosCount=0;
my $DwellTimeUntilClick=0;
my $SumDensBadQuery=0;
my $NumBackSerp=0;
my $QuerySimilarity = 0;
my $ClickCount=0;
my $QueryWOClick=0;
# sevice vars
my @clicks_pos;
my @dwell_times_until_click;
my $last_was_query=0;
my $curr_serp;
my %serp;
my @last_query_urls;

while(<TRAIN>) {
    chomp($_);
    my $line = $_;
    my @line = split('\t',$line,6);
    my $sess_id = $line[0];
    my $sess_type = $line[2];
# percentage
    $lines_num = $lines_num+1; 
    if ($lines_num % $ten_portion == 0) {
        print $lines_num/$train_set_l . "\n";
    }
# ------    
    if ($sess_type eq "M" ) {
        if ($first_time) {
            $first_time = 0;
        } else {
            # write stats
            if ($ClickCount != 0) {
                $AvgPosCount = sum(@clicks_pos)/@clicks_pos;
                $DwellTimeUntilClick = sum(@dwell_times_until_click)/@dwell_times_until_click;
            } else {
                $AvgPosCount=11;
                $DwellTimeUntilClick=-1;    
            }
            foreach my $k (keys %serp) {
                if ($serp{$k}{'clicks'} == 0) {
                    $QueryWOClick++;
                }
                undef($serp{$k}{'query'});
                undef($serp{$k})
            } 
            ($AvgPosCount, $DwellTimeUntilClick, $SumDensBadQuery, $NumBackSerp, $QuerySimilarity, $ClickCount, $QueryWOClick) = 
                (AvgPosCountRate($AvgPosCount), 
                 DwellTimeUntilClickRate($DwellTimeUntilClick),
                 SumDensBadQueryRate($SumDensBadQuery),
                 NumBackSerpRate($NumBackSerp),
                 QuerySimilarityRate($QuerySimilarity),
                 ClickCountRate($ClickCount),
                 QueryWOClickRate($QueryWOClick) 
                 );
            print OUT &make_out_s($AvgPosCount, $DwellTimeUntilClick, $SumDensBadQuery, $NumBackSerp, $QuerySimilarity, $ClickCount, $QueryWOClick);

            
            undef(%serp);
            undef(@clicks_pos);
            undef(@dwell_times_until_click);
            undef(@last_query_urls);
            $last_was_query=0;
            
            $AvgPosCount=0;
            $DwellTimeUntilClick=0;
            $SumDensBadQuery=0;
            $NumBackSerp=0;
            $QuerySimilarity=0;
            $ClickCount=0;            
            $QueryWOClick=0;
        }
        next;
    }
    if ($sess_type eq "Q") {
        $curr_serp = $line[3];
        my $query_id = $line[4];
        if (exists $query_prob{$query_id}) {
            $SumDensBadQuery += $query_prob{$query_id};
        }
        my @urls = split(/\t/,$line[5]);
        if (@last_query_urls) {
            $QuerySimilarity += similarity(\@last_query_urls, \@urls);
        }
        @last_query_urls = @urls;
        
        my %current_query;
        my %query_prop;
        $query_prop{'clicks'} = 0;
        @current_query{@last_query_urls} = (1 .. ($#urls+1));
        $query_prop{'query'} = \%current_query;
        $serp{$curr_serp} = \%query_prop;
        
        #for DwellTimeUntilClick 
        $last_was_query=1;
        
        undef($query_id);
        undef(@urls);
    } else {
        $last_was_query=0;
    }
    if ($sess_type eq "C") {
        $ClickCount += 1;
        my $serp_id = $line[3];        
        my $query_id = $line[4];
        $serp{$serp_id}{'clicks'} += 1;
        push @clicks_pos, $serp{$serp_id}{'query'}{$query_id};
        if(not $curr_serp == $serp_id ) {
             $NumBackSerp += 1;
        }
        push @dwell_times_until_click, $line[1];
        undef($serp_id);
        undef($query_id);
    }
    if($lines_num == $train_set_l) {
            # write stats
            if ($ClickCount != 0) {
                $AvgPosCount = sum(@clicks_pos)/@clicks_pos;
                $DwellTimeUntilClick = sum(@dwell_times_until_click)/@dwell_times_until_click;
            } else {
                $AvgPosCount=-1;
                $DwellTimeUntilClick=-1;    
            }
            foreach my $k (keys %serp) {
                if ($serp{$k}{'clicks'} == 0) {
                    $QueryWOClick++;
                }
                undef($serp{$k}{'query'});
                undef($serp{$k})
            } 
            ($AvgPosCount, $DwellTimeUntilClick, $SumDensBadQuery, $NumBackSerp, $QuerySimilarity, $ClickCount, $QueryWOClick) = 
                (AvgPosCountRate($AvgPosCount), 
                 DwellTimeUntilClickRate($DwellTimeUntilClick),
                 SumDensBadQueryRate($SumDensBadQuery),
                 NumBackSerpRate($NumBackSerp),
                 QuerySimilarityRate($QuerySimilarity),
                 ClickCountRate($ClickCount),
                 QueryWOClickRate($QueryWOClick) 
                 );
            print OUT &make_out_s($AvgPosCount, $DwellTimeUntilClick, $SumDensBadQuery, $NumBackSerp, $QuerySimilarity, $ClickCount, $QueryWOClick);
            
            undef(%serp);
            undef(@clicks_pos);
            undef(@dwell_times_until_click);
            undef(@last_query_urls);
            $last_was_query=0;
            
            $AvgPosCount=0;
            $DwellTimeUntilClick=0;
            $SumDensBadQuery=0;
            $NumBackSerp=0;
            $QuerySimilarity=0;
            $ClickCount=0;            
            $QueryWOClick=0;   
    }
    undef($sess_id);
    undef($line);
    undef(@line);
    undef($sess_type);
}

close(TRAIN);
#close(PATTERNS);
exit(0);
