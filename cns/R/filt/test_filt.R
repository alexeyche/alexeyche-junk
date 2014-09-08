setwd("~/prog/alexeyche-junk/cns/R/filt")

require(MASS)
source('nengo.R')

dt = 1

t_rc = 0.02 * 1000 
t_ref = 0.002 * 1000
M = 100
rate_low = 25
rate_high = 50
v_tresh = 1


n = list(v=rep(0, M), ref=rep(0,M))

encoder = sample(c(1,-1),M, replace=TRUE)

x_ts = loadMatrix("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_512",1)[1,]
x_ts =  2*(x_ts-min(x_ts))/(max(x_ts)-min(x_ts))-1
c(gain, bias) := generate_gain_and_bias(M, min(x_ts), max(x_ts), rate_low, rate_high)
gain = gain
bias = bias


spikes = NULL
for(i in 1:length(x_ts)) {
    x = x_ts[i]
    input = x * encoder * gain + bias
    
    c(n, current_spikes) := run_neurons(input, n)
    spikes = cbind(spikes, as.integer(current_spikes))    
}    

x = seq(min(x_ts), max(x_ts), length.out=100)
resp = NULL
for(xv in x) {
    r = compute_response(xv, encoder, gain, bias, 0.5)
    resp = cbind(resp, r)
}

cols = rainbow(nrow(resp))
for(ni in 1:nrow(resp)) {
    if(ni==1) {
        plot(x, resp[ni,], type="l", col=cols[ni], ylim=c(min(resp), max(resp)))
    } else {
        lines(x, resp[ni,], col=cols[ni])
    }
}
L = 100

big_id = which(rowSums(spikes) >= max(rowSums(spikes)))
sp = as.matrix(spikes[big_id[1], ])
sp_t = which(sp == 1)

x_ts = x_ts[1:L]
sp = sp[1:L]

w = as.matrix(exp(-(1:L)/10)) # default filter

filt = Vectorize(function(s, w) {
    if(s<0) return(0)
    if(s>=L) return(0)
    w[s+1]
},"s")

conv = function(x, w) {
    xv = NULL
    L = length(w)
    for(i in 1:length(x)) {    
        w_i = (i-L+1):i
        w_i = w_i[w_i>0]
        
        xv = c(xv, t(rev(w[1:length(w_i)])) %*% x[w_i])
    }
    return(xv)
}
sp_c = conv(sp, w)

plotl(sp_c)

E = sum((x_ts - sp_c)^2)

Rxx = matrix(0, nrow=L, ncol=L)
rxd = rep(0, L)
for(i in L:length(x_ts)) {
    w_i = (i-L+1):i
    
    Rxx = Rxx + x_ts[w_i] %*% t(x_ts[w_i])
    rxd = rxd + x_ts[w_i] * sp[w_i]
}
Rxx = sapply(1:nrow(Rxx), function(i) rev(Rxx[i,]))


Rxx = Rxx/(length(x_ts)-L+1)
rxd = rxd/(length(x_ts)-L+1)

w2 = ginv(Rxx) %*% rxd

sp_c2 = conv(sp, w2)

E2 = sum((x_ts - sp_c2)^2)
plotl(sp_c2)

Lx = 10000

x = sin(2*pi/8*(seq(0,Lx-1)))
x = c(1,2,3,4,5)

