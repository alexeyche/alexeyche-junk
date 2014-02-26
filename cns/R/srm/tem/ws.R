#!/usr/bin/RScript
# http://home.etf.rs/~milic/Solution_Manual/Chapter_12_exercises/Chapter_12_exercises.html

require(wavelets)
require(signal)
nf = 10

downsample = function(x, y) {
  
}

encode_mask = rep(1, nf)
encode_mask[seq(2,10, by=2)] = -1

f = wt.filter('d10')
h0 = f@h * encode_mask
h1 = f@g * encode_mask
g0 = f@h
g1 = f@g

x = c(rep(0, 100), 0.01*seq(1,100), rep(0,312))

# level 1
x0 = filter(h0, 1, x)
x1 = filter(h1, 1, x)
v0 = resample(x0, p=2, q=1)
v1 = resample(x1, p=2, q=1)

# level 2
x2 = v0
x02 = filter(h0, 1, x2)
x12 = filter(h1, 1, x2)
v02 = resample(x02, 0.5)
v12 = resample(x12, 0.p5)
v2 = v12

# level 3
x3 = v02
x03 = filter(h0, 1, x3)
x13 = filter(h1, 1, x3)
v03 = resample(x03, 0.5)
v13 = resample(x13, 0.5)
v3 = v13
v4 = v03

# synth:

# level 3
w03 = resample(v4, 2)
w13 = resample(v3, 2)
y2 = filter(g0, 1, w03) + filter(g1, 1, w13)

# level 2
w12 = c(rep(0, nf-1), v2[1:(length(v2) - (nf-1))])
w12 = resample(w12, 2)
yu2 = resample(y2, 2)

y1 = filter(g0, 1,yu2) + filter(g1, 1, w12)

# level 1

w11 = c(rep(0, 3*(nf-1)), v1[1:(length(v1) - 3*(nf-1))])
w11 = resample(w11, 2)
yu1 = resample(y1, 2)

y1 = filter(g0, 1,yu2) + filter(g1, 1, w12)

y = filter(g0, 1, yu1) + filter(g1, 1, w11)

