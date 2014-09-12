
source('nengo.R')
source('util.R')
source('serialize_to_bin.R')


dt = 1

t_rc = 0.02 * 1000 
t_ref = 0.002 * 1000
M = 50
rate_low = 0
rate_high = 50
v_tresh = 1




Xi = sample(1:10)

X = NULL
for(i in Xi) {
    X = c(X, loadMatrix("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_120",i)[1,])
}

X =  2*(X-min(X))/(max(X)-min(X))-1


M = 25
c(gain, bias) := generate_gain_and_bias(M, rate_low, rate_high)
encoder = sample(c(1,-1),M, replace=TRUE)


n = list(v=rep(0, M), ref=rep(0,M))


spikes = NULL
for(i in 1:length(X)) {
    x = X[i]
    input = x * encoder * gain + bias
    
    c(n, current_spikes) := run_neurons(input, n)
    spikes = cbind(spikes, as.integer(current_spikes))    
}

