#!/usr/bin/env perl
use strict;



sub isQuery {
    my $el = @_[0];
    if(grep {$_ eq $el} ("Q","Qbad1","Qbad2","Qbad3")) {
        return 1;
    }
    return 0;
}
sub isQueryBad {
    my $el = @_[0];
    if(grep {$_ eq $el} ("Qbad1","Qbad2","Qbad3")) {
        return 1;
    }
    return 0;
}
sub isClick {
    my $el = @_[0];
    if(grep {$_ eq $el} ("C0","C1")) {
        return 1;
    }
    return 0;
}

my $file = $ARGV[0];
open(PATT_OUT, "<$file");

while(<PATT_OUT>) {
    my @line = split(',',$_);
    my $last = "";
    my @stats;
    my $stat = "";
    my $new_stat = "";
    foreach my $el (@line) {
        chomp($el);
        if($last) {
            if(isQuery($el) and isQuery($last)) {
#                 if(isQueryBad($el) or isQueryBad($last)) {
#                     $new_stat = "Qbadn";
#                 } else {
#                     if(not $stat eq "Qbadn") {
                        $new_stat = "Qn";
#                     }
#                 }
            }
            if(isClick($el) and isQuery($last)) {
                $new_stat = "QCn";              
            }
            if(isClick($el) and isClick($last)) {
                $new_stat = "Cn";              
            } 
            if(not $stat eq $new_stat) {
                if($stat) {
                    push @stats, $stat;  # saving already old stat
                }
                $stat = $new_stat;
            }
        }
        $last = $el;
    }
    if(not $stat and not @stats) {
        if (isQuery($last)) {
            push @stats, 'Q';
        }
    } else {
        push @stats, $stat;
    }
    
#    print PATT_CALC join(",",@line) . " = ". join(",", @stats)."\n";
    print join(",", @stats)."\n";
}
close(PATT_OUT);
