

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


eval = function(train_net, test_net, sim,  kernel_sigma) {
    if(train_net$duration != test_net$duration) {
        cat("Error!\n")
        q()
    }
    s$sim(eval_sim_opt, constants, train_net)
    s$sim(eval_sim_opt, constants, test_net)
    train_resp = train_net$getResponces()
    test_resp = test_net$getResponces()

    #resp = list()
    #resp[1:length(train_resp)] = train_resp
    #resp[ (length(train_resp)+1):(length(train_resp)+length(test_resp)) ] = test_resp
    #Ksim = simMatrix(resp, train_net$duration, kernel_sigma)
    
    c(r, confm_data) := ucr_test(train_resp, test_resp, cross_entropy_alg, FALSE)
    labs = unique(train_net$labels)
    confm = matrix(0, length(labs), length(labs))
    for(d in confm_data) {
        confm[d$pred, d$true] = confm[d$pred, d$true] + 1
    }
    cat("Conf:\n")
    print(confm)    
    return(r)
}
