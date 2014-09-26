#!/usr/bin/perl
use strict;
use warnings;


my $in_lat_sec = 0;
#my $in_lat_sec_reg = "/\(begin|end){(equation|figure)}/";
while(my $l = <>) {
    if($l =~ m/\\(begin|end){(align|equation|figure)}/) {
        if($1 eq "begin") {
            $in_lat_sec = 1;
        } 
        elsif($1 eq "end") {
            $in_lat_sec = 0;
            next;
        }
    }
    next if $in_lat_sec;
    $l =~ s/[ ]?\\[a-zA-Z]+(\[[,a-zA-Z0-9]+\])*({[-\\\/{}.,: _a-zA-Z0-9]+})*//g;  # \asdasdFAS{asdasd}
    $l =~ s/[ ]?\$.*\$//g;      # $ formula $
    $l =~ s/\\\\//g;        # \\
    $l =~ s/{(.*)}/$1/g;    # {Name of section}
    print $l;
}
