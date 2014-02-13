#!/usr/bin/env Rscript

run_net <- function(input_neurons, net_neurons, ro, verbose=TRUE) {
    patterns = input_neurons$patterns
    lengths = sapply(patterns, function(p) length(p$data))
    stopifnot(all(lengths == lengths[1]))
    
    id_m = input_neurons$ids
    id_n = net_neurons$all_ids
    
    net = list()
    net[id_m] = patterns[[1]]$data
    net[id_n] = -Inf
    
    loss = NULL  
    stable = NULL  
    sim_opt = list(T0=ro$T0, Tmax=ro$Tmax, dt=ro$dt, saveStat=ro$collect_stat, 
                   seed=ro$seed_num, learn=TRUE)
    
    for(ep in ro$start_epoch:ro$epochs) {
        warn_count=0
        net_all = list()    
        for(id_patt in 1:length(patterns)) {
            net[id_m] = patterns[[id_patt]]$data
            net[id_n] = -Inf
                
            sim_out = net_neurons$sim(sim_opt, net)
            
            if(verbose)
                cat("epoch: ", ep, ", pattern # ", id_patt, "\n", sep="")
                
            pic_filename = sprintf("%s/run_ep%s_patt%s_label%s.png", dir, ep, id_patt, patterns[[id_patt]]$label)
            plot_run_status(net, net_neurons, sim_out, pic_filename, 
                            sprintf("epoch %d, pattern %d, class %d", 
                                    ep, id_patt, patterns[[id_patt]]$label))
#            if(open_plots) system(sprintf("eog -w %s 1>/dev/null 2>/dev/null",pic_filename), ignore.stdout=TRUE, ignore.stderr=TRUE, wait=FALSE)
            
            if(sum(sapply(net[id_n], length))> ((ro$Tmax-ro$T0)*length(id_n)/2)) {
              warn_count = warn_count + 1
              if(warn_count>5) {
                  return(c(1.0))
              }         
            }     
            net_all[[id_patt]] = list(data=net[id_n], label=patterns[[id_patt]]$label)
        }
    
        model_file = sprintf("%s/%s_%dx%d_%d", dir, data, M, N, ep)
        W = get_weights_matrix(net_neurons$l)
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
