#!/usr/bin/env bash
set -x
g++-8 sketch.cpp liblbfgs/lib/lbfgs.c -I"liblbfgs/include" -msse2 -O3 -o sketch