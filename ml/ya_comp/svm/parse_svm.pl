#!/usr/bin/env perl

use strict;
use Data::Dumper;
use List::Util qw(sum);
use POSIX;

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
        my $sym = " ";
        if($i == $ind) {
            $sym = "\n"            
        }
        my $id = $i+1;
        $str = $str ."$id:". $arr[$i] . $sym;             
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

my $file = $ARGV[0];
open(TRAIN, "<$file");
my $class = 0;
my $test_set = 0;
if($ARGV[1] eq "-t") {
    $test_set=1;   
}
my $first_time=1;

my $train_set_l=`cat $file | wc -l`;
#my $train_set_l=52001965;
my $ten_portion = floor($train_set_l/10);
my $lines_num = 0;

# features
my $AvgPosCount=0;
my $DwellTimeUntilClick=0;
my $SumDensBadQuery=0;
my $NumBackSerp=0;
my $QuerySimilarity = 0;
my $Click2Query=0;
my $QueryWOClick=0;
my $UserBack=0;
my $WholeSessionTime=0;
my $AvgQueryQueryDwellTime=0;
my $AvgClickClickDwellTime=0;
# sevice vars
my @clicks_pos;
my @dwell_times_until_click;
my $last_was_query=0;
my $last_was_click=0;
my $curr_serp;
my %serp;
my @last_query_urls;
my $user_id;
my $query_count=0;
my $click_count=0;
my @query_query_dwell_time;
my @click_click_dwell_time;
my $timeline=0;
my $last_sess;
while(<TRAIN>) {
    chomp($_);
    my $line = $_;
    my @line = split('\t',$line,6);
    my $sess_id = $line[0];
    my $sess_type = $line[2];
    if( ($test_set == 1) && ( $sess_type ne "M") ) {
        $class = $sess_id;
    }
# percentage
    $lines_num = $lines_num+1; 
    if ($lines_num % $ten_portion == 0) {
        print STDERR $lines_num/$train_set_l . "\n";
    }
# ------    
    if ($sess_type eq "M" ) {
        if ($first_time) {
            $user_id = $line[3];
            $first_time = 0;
        } else {
            # write stats
            if ($click_count != 0) {
                $AvgPosCount = sum(@clicks_pos)/@clicks_pos;
                $DwellTimeUntilClick = sum(@dwell_times_until_click)/@dwell_times_until_click;
            } else {
                $AvgPosCount=11;
                $DwellTimeUntilClick=-1;    
            }
            if(@query_query_dwell_time) {
                $AvgQueryQueryDwellTime = sum(@query_query_dwell_time)/@query_query_dwell_time;
            }
            if(@click_click_dwell_time) {
                $AvgClickClickDwellTime = sum(@click_click_dwell_time)/@click_click_dwell_time;
            }
            foreach my $k (keys %serp) {
                if ($serp{$k}{'clicks'} == 0) {
                    $QueryWOClick++;
                }
                undef($serp{$k}{'query'});
                undef($serp{$k})
            } 
            $Click2Query = $click_count/$query_count;
            
            print "$class " . &make_out_s($AvgPosCount, $DwellTimeUntilClick, $SumDensBadQuery, $NumBackSerp, $QuerySimilarity, $Click2Query, $QueryWOClick, $AvgClickClickDwellTime, $AvgQueryQueryDwellTime);
            #            print "$class " . &make_out_s($AvgPosCount, $DwellTimeUntilClick, $SumDensBadQuery, $NumBackSerp, $QuerySimilarity, $Click2Query, $QueryWOClick, $WholeSessionTime, $AvgClickClickDwellTime, $AvgQueryQueryDwellTime);
            
#            if($line[3] == $user_id) {
#                $UserBack++;
#            } else {
#                $UserBack=0;
#            }          

            
            undef(%serp);
            undef(@clicks_pos);
            undef(@dwell_times_until_click);
            undef(@last_query_urls);
            undef(@query_query_dwell_time);
            undef(@click_click_dwell_time);
            $last_was_query=0;
            $last_was_click=0;
            $click_count=0;            
            $query_count=0;
            $timeline=0;            

            $AvgPosCount=0;
            $DwellTimeUntilClick=0;
            $SumDensBadQuery=0;
            $NumBackSerp=0;
            $QuerySimilarity=0;
            $QueryWOClick=0;
            $WholeSessionTime=0;
            $AvgQueryQueryDwellTime=0;
            $AvgClickClickDwellTime=0;
        }
        next;
    }
    if ($sess_type eq "Q") {
        if($last_was_query) {
            my $delta_query_time = $line[1] - $WholeSessionTime;
            push @query_query_dwell_time, $delta_query_time;            
        }
        $WholeSessionTime = $line[1];
        $query_count += 1;
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
        if($last_was_click) {
            my $delta_click_time = $line[1] - $WholeSessionTime;
            push @click_click_dwell_time, $delta_click_time;            
        }
        $WholeSessionTime = $line[1];
        $click_count += 1;
        my $serp_id = $line[3];        
        my $query_id = $line[4];
        $serp{$serp_id}{'clicks'} += 1;
        push @clicks_pos, $serp{$serp_id}{'query'}{$query_id};
        if(not $curr_serp == $serp_id ) {
             $NumBackSerp += 1;
        }
        push @dwell_times_until_click, $line[1];
        
        $last_was_click=1;

        undef($serp_id);
        undef($query_id);
    } else {
        $last_was_click=0;
    }
    if($sess_type eq 'S') {
        $WholeSessionTime = $line[1];
    }
#    if($lines_num == $train_set_l) {
            # write stats
            # write stats
 #    }
#    undef($sess_id);
#    undef($line);
#    undef(@line);
#    undef($sess_type);
}
           
            if ($click_count != 0) {
                $AvgPosCount = sum(@clicks_pos)/@clicks_pos;
                $DwellTimeUntilClick = sum(@dwell_times_until_click)/@dwell_times_until_click;
            } else {
                $AvgPosCount=11;
                $DwellTimeUntilClick=-1;    
            }
            if(@query_query_dwell_time) {
                $AvgQueryQueryDwellTime = sum(@query_query_dwell_time)/@query_query_dwell_time;
            }
            if(@click_click_dwell_time) {
                $AvgClickClickDwellTime = sum(@click_click_dwell_time)/@click_click_dwell_time;
            }
            foreach my $k (keys %serp) {
                if ($serp{$k}{'clicks'} == 0) {
                    $QueryWOClick++;
                }
                undef($serp{$k}{'query'});
                undef($serp{$k})
            } 
            $Click2Query = $click_count/$query_count;
            
            print "$class " . &make_out_s($AvgPosCount, $DwellTimeUntilClick, $SumDensBadQuery, $NumBackSerp, $QuerySimilarity, $Click2Query, $QueryWOClick, $WholeSessionTime, $AvgClickClickDwellTime, $AvgQueryQueryDwellTime);
            #            print "$class " . &make_out_s($AvgPosCount, $DwellTimeUntilClick, $SumDensBadQuery, $NumBackSerp, $QuerySimilarity, $Click2Query, $QueryWOClick, $WholeSessionTime, $AvgClickClickDwellTime, $AvgQueryQueryDwellTime);
            
#            if($line[3] == $user_id) {
#                $UserBack++;
#            } else {
#                $UserBack=0;
#            }          

            
            undef(%serp);
            undef(@clicks_pos);
            undef(@dwell_times_until_click);
            undef(@last_query_urls);
            undef(@query_query_dwell_time);
            undef(@click_click_dwell_time);
            $last_was_query=0;
            $last_was_click=0;
            $click_count=0;            
            $query_count=0;
            $timeline=0;            

            $AvgPosCount=0;
            $DwellTimeUntilClick=0;
            $SumDensBadQuery=0;
            $NumBackSerp=0;
            $QuerySimilarity=0;
            $QueryWOClick=0;
            $WholeSessionTime=0;
            $AvgQueryQueryDwellTime=0;
            $AvgClickClickDwellTime=0;

close(TRAIN);
exit(0);
