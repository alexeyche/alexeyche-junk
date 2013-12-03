#!/usr/bin/env bash
set -x
if [ $1 == "-t" ]; then
    [ -f parse_out.test ] &&  ( rm -rf parse_out.test && rm -rf parse_out.test.norm )
    ./parse.pl ./dataset/test > parse_out.test
    ./proc norm parse_out.test parse_out.test.norm
else
    [ -f parse_out ] &&  ( rm -rf parse_out && rm -rf parse_out.norm )
    ./parse.pl ./dataset/train > parse_out
    ./proc norm parse_out parse_out.norm
fi
