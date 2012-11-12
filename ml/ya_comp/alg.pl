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

my @arr1=(1,2,3,4,5,6);
#my @arr2=(3,2,1,4,5,6);
#my @arr2=(5,4,6,1,3,2);
my @arr2=('a','b','c','d');

my $l = levenshtein(\@arr1,\@arr2);
print Dumper($l);

