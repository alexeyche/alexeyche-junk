#!/usr/bin/env python

import math

n = 10
rec = ["0"]*n
m = 5000

n_i=0
sin_scale = (-math.pi, math.pi)
sin1_a = 0
sin1_inc = math.pi/10
for i in xrange(0,m):
    sin1_a += sin1_inc
    r = list(rec)
    r[n_i] = "1"
    n_i+=1
    if n_i>=10:
        n_i=0
    print ",".join(r)        
