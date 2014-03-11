
eval_patterns = function(patterns, gr1, neurons, s, duration) {
    responces = list()
    for(pi in 1:length(patterns)) {
        p = patterns[[pi]]$data
        net = list()
        net[gr1$ids()] = p
        net[neurons$ids()] = blank_net(N)
        
        sim_opt = list(T0=0, Tmax=duration, dt=dt, saveStat=FALSE, learn=FALSE, determ=TRUE)
        s$sim(sim_opt, constants, net)
        responces[[pi]] = list(data=net[neurons$ids()], label=patterns[[pi]]$label)
    } 
    return(responces)
}

simMatrix = function(responces, duration, sigma) {
    kernel_options = list(T0=0,Tmax=duration, quad=256, sigma=sigma)
    Ksim = matrix(0, length(responces), length(responces))
    for(ri in 1:length(responces)) {
        for(rj in 1:length(responces)) {
            if(ri!=rj) {
                Ksim[ri,rj] = sum(kernelCrossCorr(responces[[ri]], responces[[rj]], kernel_options)$data^2)
            }
        }
    }
    return(Ksim)
}


eval = function(patterns, gr1, neurons, s, duration, sigma) {
    responces = eval_patterns(patterns, gr1, neurons, s, duration)
    Ksim = simMatrix(responces, duration, sigma)
    cat("Ksim:\n")
    print(Ksim)    
    return(sum(Ksim^2))
}