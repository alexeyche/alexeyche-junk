

#require(Rsnn)

getCorrRate = function(p, lambda) {
    if(p == 1) return(2*lambda)
    p * lambda/(1-p) 
}

gen_correlated_poisson = function(N, mean_rate=10, correllated_neurons=50, group_size=2, correlation=0.5, len=10000, binary=FALSE) {
    
    corr_rate = mean_rate*getCorrRate(correlation, 1)/2
    decorr_rate = mean_rate*(1-getCorrRate(correlation, 1)/2)
    
    rates = c(rep(decorr_rate, correllated_neurons), rep(mean_rate, N-correllated_neurons))
    
    corr_spikes = vector("list",N)
    for(ni in seq(1, correllated_neurons, by=group_size)) {
        pair_corr_spikes = rpois(len, corr_rate/1000.0)
        for(gi in 0:(group_size-1)) {
            corr_spikes[[ni+gi]] = pair_corr_spikes            
        }
    }
    
    
    net = vector("list",N)
    net_b = NULL
    for(ni in 1:N) {
        spikes_bin = rpois(len, rates[ni]/1000.0)  # ms precision
        if(!is.null(corr_spikes[[ni]])) spikes_bin = spikes_bin + corr_spikes[[ni]]
        net_b = rbind(net_b, spikes_bin)
        net[[ni]] = which(spikes_bin == 1)
    }
    if(binary) {
        return(net_b)    
    } else {
        return(net)
    }
}

gen_poisson = function(N, rate=10, len=10000, binary=FALSE)  {
    net_b = t(sapply(1:N, function(x) rpois(len, rate/1000)))
    if(binary) {
        net_b
    } else {  
        lapply(1:N, function(ni) which(net_b[ni,]>= 1))
    }
    
}
    


