
source('nengo.R')
source('util.R')

Xi = sample(1:100)

X = NULL
for(i in Xi) {
    X = c(X, loadMatrix("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_120",i)[1,])
}

X =  2*(X-min(X))/(max(X)-min(X))-1

M = 25
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

