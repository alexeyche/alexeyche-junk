#!/usr/bin/env Rscript


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
simCubeCC = function(responces, duration, sigma) {
    Ksim = array(0, dim=c(length(responces[[1]]$data), length(responces), length(responces)))
    for(ri in 1:length(responces)) {
        for(rj in 1:length(responces)) {
            if(ri!=rj) {
                Ksim[,ri,rj] = kernelCrossCorr(responces[[ri]], responces[[rj]], kernel_options)$data/length(responces[[ri]]$data)
            }
        }
    }
    return(Ksim)
}

simCube = function(responces, duration, sigma) {
    Ksim = array(0, dim=c(length(responces[[1]]$data), length(responces), length(responces)))
    for(ri in 1:length(responces)) {
        for(rj in 1:length(responces)) {
            if(ri!=rj) {
                Ksim[,ri,rj] = kernelCrossEntropy(responces[[ri]], responces[[rj]], kernel_options)$data/length(responces[[ri]]$data)
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
    
    resp_l = c(train_net_ev$labels, test_net_ev$labels)
    ulab = unique(resp_l)
    resp = list()
    resp[1:length(train_resp)] = train_resp
    resp[ (length(train_resp)+1):(length(train_resp)+length(test_resp)) ] = test_resp
    
    KsimCC = simCubeCC(resp, train_net_ev$duration, kernel_sigma)
    sim = NULL
    discr = NULL
    
    for(ri in 1:length(resp)) {
        cur_l = resp_l[ri]
        id_l = which(cur_l == resp_l)  
        id_not_l = which(cur_l != resp_l)
        v = rowSums(KsimCC[,ri, id_l])/(length(id_l)-1)
        v_nl = rowSums(KsimCC[,ri, id_not_l])/(length(id_not_l))     
        sim = cbind(sim, v)
        discr = cbind(discr, v_nl)        
    }
    sim = rowMeans(sim)
    discr = rowMeans(discr)
    c(r, confm_data) := ucr_test(train_resp, test_resp, cross_corr_alg, FALSE)
    labs = unique(train_net_ev$labels)
    confm = matrix(0, length(labs), length(labs))
    for(d in confm_data) {
        confm[d$pred, d$true] = confm[d$pred, d$true] + 1
    }
    cat("Conf:\n")
    print(confm)    
    return(list(sim, discr, r))
}
