

eval_sim_opt = list(dt=dt, saveStat=FALSE, learn=FALSE, determ=TRUE)




simMatrix = function(responces, duration, sigma) {
    
    Ksim = matrix(0, length(responces), length(responces))
    for(ri in 1:length(responces)) {
        for(rj in 1:length(responces)) {
            if(ri!=rj) {
                Ksim[ri,rj] = sum(kernelCrossEntropy(responces[[ri]], responces[[rj]], kernel_options)$data^2)/length(responces[[ri]]$data)
            }
        }
    }
    return(Ksim)
}

simMatrixCC = function(responces, duration, sigma) {
    
    Ksim = matrix(0, length(responces), length(responces))
    for(ri in 1:length(responces)) {
        for(rj in 1:length(responces)) {
            if(ri!=rj) {
                Ksim[ri,rj] = sum(kernelCrossCorr(responces[[ri]], responces[[rj]], kernel_options)$data^2)/length(responces[[ri]]$data)
            }
        }
    }
    return(Ksim)
}

eval = function(train_net_ev, test_net_ev, sim,  kernel_sigma) {
    if(train_net_ev$duration != test_net_ev$duration) {
        cat("Error!\n")
        q()
    }
    s$sim(eval_sim_opt, constants, train_net_ev)
    s$sim(eval_sim_opt, constants, test_net_ev)
    train_resp = train_net_ev$getResponces()
    test_resp = test_net_ev$getResponces()

#     resp = list()
#     resp[1:length(train_resp)] = train_resp
#     resp[ (length(train_resp)+1):(length(train_resp)+length(test_resp)) ] = test_resp
#     KsimCC = simMatrixCC(resp, train_net$duration, kernel_sigma)
#     Ksim = simMatrix(resp, train_net$duration, kernel_sigma)
    
    c(r, confm_data) := ucr_test(train_resp, test_resp, cross_corr_alg, FALSE)
    labs = unique(train_net_ev$labels)
    confm = matrix(0, length(labs), length(labs))
    for(d in confm_data) {
        confm[d$pred, d$true] = confm[d$pred, d$true] + 1
    }
    cat("Conf:\n")
    print(confm)    
    return(r)
}
