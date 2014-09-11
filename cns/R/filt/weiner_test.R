

require(MASS)

Lx = 10000

L=2

x = sin(2*pi/8*(seq(0,Lx-1)))

r = xcorr(x,len=L-1)/Lx 
R = toeplitz(r[L:1], r[L:(2*L-1)])

cf=c(1,-1)

d = filter(x, cf, circular=TRUE)

rdx = xcorr(d, x, L-1)/Lx
p = rdx[L:(2*L-1)]

w_opt = ginv(R) %*% p

xs = filter(d, rev(w_opt), circular=TRUE)

hist(x-xs)
