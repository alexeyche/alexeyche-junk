#!/usr/bin/env Rscript

run_net <- function(input_neurons, layers, run_options, open_plots = FALSE, model_descr=NULL, verbose=TRUE) {
    net_neurons = layers
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
  
    for(ep in run_options$start_epoch:run_options$epochs) {
        warn_count=0
        reward = NULL
        net_all = list()    
        
        for(id_patt in 1:length(patterns)) {
            net[id_m] = patterns[[id_patt]]$data
            net[id_n] = -Inf
            run_options$target_set$label = patterns[[id_patt]]$label
                
            c(net, net_neurons, stat, grad) := run_srm(net_neurons, net, run_options)
            c(grad, spikes_survived, cur_reward) := grad
            
            reward = c(reward, cur_reward)
            if(verbose)
                cat("epoch: ", ep, ", pattern # ", id_patt, ", spikes survived: ", spikes_survived,"\n", sep="")
                
            pic_filename = sprintf("%s/run_ep%s_patt%s_label%s.png", dir, ep, id_patt, patterns[[id_patt]]$label)
            plot_run_status(net, net_neurons, grad, loss, reward, pic_filename, sprintf("epoch %d, pattern %d, class %d", ep, id_patt, patterns[[id_patt]]$label))
            if(open_plots) system(sprintf("eog -w %s 1>/dev/null 2>/dev/null",pic_filename), ignore.stdout=TRUE, ignore.stderr=TRUE, wait=FALSE)

            
            if(sum(sapply(net[id_n], length))> ((run_options$Tmax-run_options$T0)*length(id_n)/2)) {
              warn_count = warn_count + 1
              if(warn_count>5) {
                  return(c(1.0))
              }         
            }     
            net_all[[id_patt]] = list(data=net[id_n], label=patterns[[id_patt]]$label)
        }
    
        if(run_options$reward_learning) {
            mean_activity = get_mean_activity_classes(net_all, run_options)
            err = NULL
            for(sp in net_all) {
                err = c(err, reward_func(sp, mean_activity))
            }           
            print(mean(err))
            mean_error = mean(err)
            run_options$mean_activity_stat = list(act=mean_activity, mean_error=mean_error)
        } 
        model_file = sprintf("%s/%s_%dx%d_%d", dir, data, M, N, ep)
        W = get_weights_matrix(net_neurons$l)
        saveMatrixList(model_file, list(W))
    
        if((! is.null(run_options$test_function))&&(ep %% run_options$test_run_freq == 0)) {
#             o_train = evalNet(patterns, run_options, constants, net_neurons$l)
#             o_test = evalNet(run_options$test_patterns, run_options, constants, net_neurons$l)
#             
#             t_out <- run_options$test_function(o_train$spikes, o_test$spikes)
#             
#             if((length(stable)>0)&&(t_out$stable>(5*stable[length(stable)]))) {
#               t_out$stable <- stable[length(stable)]
#             }
#             if(!is.null(t_out$stable)) { 
#                 stable <- c(stable, t_out$stable)      
#             }
#             loss <- c(loss, t_out$loss)
#             system( sprintf("echo %s > %s/%d.log", t_out$loss, dir, ep))
        }
    }
    return(loss)
}
