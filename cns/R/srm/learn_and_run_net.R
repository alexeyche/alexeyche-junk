#!/usr/bin/env Rscript

run_net <- function(input_neurons, n, ro, verbose=TRUE) {
    patterns = input_neurons$patterns
    lengths = sapply(patterns, function(p) length(p$data))
    stopifnot(all(lengths == lengths[1]))
    
    id_m = input_neurons$ids
    id_n = n$ids()
    
    net = list()
    net[id_m] = patterns[[1]]$data
    net[id_n] = blank_net(n$len())
    
    loss = NULL  
    stable = NULL  
    
    sim_opt = list(T0=ro$T0, Tmax=ro$Tmax, dt=ro$dt, saveStat=ro$collect_stat, 
                   seed=ro$seed_num, learn=TRUE)
    
    s = SIMClass$new(list(n)) 
    for(ep in ro$start_epoch:ro$epochs) {
        warn_count=0
        net_all = list()    
        for(id_patt in 1:length(patterns)) {
            net[id_m] = patterns[[id_patt]]$data
            net[id_n] = blank_net(n$len())
            
            s$sim(sim_opt, constants, net)
            
            if(verbose)
                cat("epoch: ", ep, ", pattern # ", id_patt, "\n", sep="")
                
            pic_filename = sprintf("%s/run_ep%s_patt%s_label%s.png", dir, ep, id_patt, patterns[[id_patt]]$label)
            plot_run_status(net, n, loss, pic_filename, 
                            sprintf("epoch %d, pattern %d, class %d", ep, id_patt, patterns[[id_patt]]$label))
            
            if(sum(sapply(net[id_n], length))> ((ro$Tmax-ro$T0)*length(id_n)/2)) {
              warn_count = warn_count + 1
              if(warn_count>5) {
                  return(c(1.0))
              }         
            }     
            net_all[[id_patt]] = list(data=net[id_n], label=patterns[[id_patt]]$label)
        }
    
        model_file = sprintf("%s/%s_%dx%d_%d", dir, data, M, N, ep)
        W = n$Wm()
        saveMatrixList(model_file, list(W))
    
        if((! is.null(ro$test_function))&&(ep %% ro$test_run_freq == 0)) {
            #o_train = evalNet(patterns, ro, constants, net_neurons$l)
            #o_test = evalNet(ro$test_patterns, ro, constants, net_neurons$l)
            
            #curloss <- ro$test_function(o_train$spikes, o_test$spikes)
            
            #loss <- c(loss, curloss)
            #system( sprintf("echo %s > %s/%d.log", curloss, dir, ep))
        }
    }
    return(loss)
}
