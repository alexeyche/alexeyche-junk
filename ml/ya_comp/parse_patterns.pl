#!/usr/bin/env perl

use strict;
use Data::Dumper;
use List::Util qw(sum);
use IO::Handle '_IOLBF';

sub uniq {
        return keys %{{ map { $_ => 1 } @_ }};
}

# load query classes
open(QCLASS, '<q_class');
my %query_class;
while(<QCLASS>) {
    my @line = split('\t',$_);
    $query_class{$line[0]} = $line[2];    
}
close(QCLASS);

my $file = $ARGV[0];
open(TRAIN, "<$file");

my $first_time=1;

my $train_set_l=52001965;
my $lines_num = 0;

# variables of common use
my @pattern; 
my $curr_serp;
my %serp;
my $switch_detected=1;
while(<TRAIN>) {
    chomp($_);
    my $line = $_;
    my @line = split('\t',$line,6);
    my $sess_id = $line[0];
    my $sess_type = $line[2];
# percentage
    $lines_num = $lines_num+1; 
    if ($lines_num % 5000000 == 0) {
        print STDERR $lines_num/$train_set_l . "\n";
    }
# ------    
    if ($sess_type eq "M") {
        if ($first_time) {
            $first_time = 0;
        } else {
            # write stats
            if ($switch_detected) {  
                print join(',', @pattern) . "\n";
            }    
            undef(@pattern);
            foreach my $k (keys %serp) {
                undef($serp{$k});
            }
            undef(%serp);
        }
        next;
    }
    if ($sess_type eq "Q") {
        $curr_serp = $line[3];
        my $query_id = $line[4];
        if (exists $query_class{$query_id}) {
            my $cl = $query_class{$query_id};
            chomp($cl);
            push @pattern, "Q".$cl;
            undef($cl);
        } else {
            push @pattern, "Q";
        }
        my %current_query;
        my @urls = split(/\t/,$line[5]);
        @current_query{@urls} = (1 .. ($#urls+1));
        $serp{$curr_serp} = \%current_query;
        undef($query_id);
        undef(@urls);
    }
    if ($sess_type eq "C") {
        my $serp_id = $line[3];        
        my $query_id = $line[4];
        my $click_pos = $serp{$serp_id}{$query_id};
        my $click_class = "0";
        if ($click_pos<4) {
           $click_class = "1"; 
        }
        if( $curr_serp == $serp_id ) {
            push @pattern, "C" . $click_class;             
        } else {
             push @pattern, "Cb" . $click_class; 
        }
        undef($serp_id);
        undef($query_id);
        undef($click_pos);
    }
    if ($sess_type eq "S") {
#        $switch_detected = 1;
#        push @pattern, "S";
    }
    undef($sess_id);
    undef($line);
    undef(@line);
    undef($sess_type);
}
if ($switch_detected) {  
    print join(',', @pattern) . "\n";
}
close(TRAIN);
exit(0);
