#setwd("~/prog/spike_code_project")
source('util.R')
source('serialize_to_bin.R')
source('ucr_ts.R')
source('nengo.R')
source('plot_funcs.R')
dt = 1

t_rc = 0.02 * 1000 
t_ref = 0.002 * 1000
M = 50
rate_low = 25
rate_high = 50
v_tresh = 1


n = list(v=rep(0, M), ref=rep(0,M))

encoder = sample(c(1,-1),M, replace=TRUE)

X = loadMatrix("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_120",1)[1,]
X =  2*(X-min(X))/(max(X)-min(X))-1
c(gain, bias) := generate_gain_and_bias(M, rate_low, rate_high)
gain = gain
bias = bias


spikes = NULL
for(i in 1:length(X)) {
    x = X[i]
    input = x * encoder * gain + bias
    
    c(n, current_spikes) := run_neurons(input, n)
    spikes = cbind(spikes, as.integer(current_spikes))    
}    

x = seq(min(X), max(X), length.out=100)
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



