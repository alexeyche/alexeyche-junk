#!/usr/bin/env bash
set -x
if [ $1 == "-t" ]; then
    if [ ! -f patterns_out.test ]; then
        ./parse_patterns.pl dataset/test > patterns_out.test
    fi    
    if [ ! -f episode_out.test ]; then 
        rm -rf ./episode_out.test*
        ./episode_analyser.pl patterns_out.test > episode_out.test
    fi  
else
    if [ ! -f patterns_out ]; then
        ./parse_patterns.pl dataset/train > patterns_out
    fi    
    if [ ! -f episode_out ]; then 
        rm -rf ./episode_out*
        ./episode_analyser.pl patterns_out > episode_out
        ./freq_analyser.pl episode_out > episode_out.freq
        sort -t '	' -k 2 -n -r ./episode_out.freq > episode_out.freq.sort
    fi  
fi
