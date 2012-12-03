#!/usr/bin/env bash
set -x
[ -f patterns_out ] && rm -rf patterns_out
./parse_patterns.pl ./dataset/train > patterns_out
[ -f patterns_stat ] && rm -rf patterns_stat
./freq_analyser.pl ./patterns_out > patterns_stat
[ -f patterns_stat.sort ] && rm -rf patterns_stat.sort
sort -t '	' -n -k 2 -r ./patterns_stat > patterns_stat.sort
