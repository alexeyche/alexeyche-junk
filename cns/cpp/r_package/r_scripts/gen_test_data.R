

require(Rsnn)

N = 100

mean_rate = 10 # Hz

correllated_neurons = 50

correlation = 0.3

getCorrRate = function(p, lambda) {
    p * lambda/(1-p) 
}
corr_rate = mean_rate*getCorrRate(correlation, 1)
decorr_rate = mean_rate*(1-getCorrRate(correlation, 1))

rates = c(rep(decorr_rate, correllated_neurons), rep(mean_rate, N-correllated_neurons))

len = 10000 # ms

corr_spikes = vector("list",N)
for(ni in seq(1, correllated_neurons, by=2)) {
    pair_corr_spikes = rpois(len, corr_rate/1000.0)
    corr_spikes[[ni]] = pair_corr_spikes
    corr_spikes[[ni+1]] = pair_corr_spikes    
}




net = vector("list",N)
net_b = NULL
for(ni in 1:N) {
    spikes_bin = rpois(len, rates[ni]/1000.0)  # ms precision
    if(!is.null(corr_spikes[[ni]])) spikes_bin = spikes_bin + corr_spikes[[ni]]
    net_b = rbind(net_b, spikes_bin)
    net[[ni]] = which(spikes_bin == 1)
}

prast(net)