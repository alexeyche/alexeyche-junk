#!/usr/bin/env perl

use strict;
use List::Util qw[min max];
use Data::Dumper;

#int LevenshteinDistance(string s, string t)
#{
#  int len_s = length(s), len_t = length(t), cost = 0
#
#  if(s[0] != t[0]) then cost = 1
#
#  if(len_s == 0) then      return len_t
#  elseif(len_t == 0) then  return len_s
#  else                     return minimum(LevenshteinDistance(s[1..len_s-1], t) + 1,
#                                          LevenshteinDistance(s, t[1..len_t-1]) + 1,
#                                          LevenshteinDistance(s[1..len_s-1], t[1..len_t-1]) + cost)
#}



sub levenshtein {
    my ($ref1, $ref2) = @_;
    my @ar1 = @{ $ref1 };
    my @ar2 = @{ $ref2 };
 
    my @dist;
    $dist[$_][0] = $_ foreach (0 .. $#ar1);
    $dist[0][$_] = $_ foreach (0 .. $#ar2);
    print Dumper(\@dist);
    foreach my $i (1 .. $#ar1){
        foreach my $j (1 .. $#ar2){
            print $ar1[$i - 1] . " " . $ar2[$j - 1];
            my $cost = $ar1[$i - 1] == $ar2[$j - 1] ? 0 : 1;
            print " $cost\n";
            $dist[$i][$j] = min(
                        $dist[$i - 1][$j] + 1, 
                        $dist[$i][$j - 1] + 1, 
                        $dist[$i - 1][$j - 1] + $cost );
        }
    }
 
    return \@dist;
}

sub my_levenstein {
    my @ar1=(1,2,3,4,5,6);
    #my @ar2=(1,2,3,4,6,7);
    #my @ar2=(3,2,1,4,5,6);
    #my @ar2=(1,1,1,1,1,1);
    my @ar2=(1,3);
    my @dist;
    foreach my $i (0 .. $#ar1) {
        foreach my $j (0 .. $#ar2) {
            $dist[$i][$j] = 0;
        }
    }
    print Dumper(\@dist);
    #exit(0);
    my @sums;
    foreach my $i (0 .. $#ar1) {
        foreach my $j (0 .. $#ar2) {
            if ($ar1[$i] == $ar2[$j]) {
                if($i == $j) {
                    $dist[$i][$j] = 1;
                } else {
                    $dist[$i][$j] = 1 -abs($i-$j)/($#ar2+1);
                }    
            } else {
                $dist[$i][$j] = 0;
            }
        }
        my $sum = 0;
        foreach my $j (0 ..$#ar2) {
            $sum = $sum + $dist[$i][$j];        
        }
        push @sums, $sum/($#ar2+1) . "\n";
    }
    my $mega_sum=0;
    foreach(@sums) {
       $mega_sum = $mega_sum + $_; 
    }
    print $mega_sum . "\n";


    #foreach my $i (0 .. $#ar1) {
    #    foreach my $j (0 .. $#ar2) {
    #        $dist[$i][$j] = 0;
    #    }
    #}
    #my $l = levenshtein(\@arr1,\@arr2);
    print Dumper(\@dist);
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

1;        
