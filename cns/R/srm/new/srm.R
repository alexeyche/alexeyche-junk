
run_srm <- function(net_neurons, net, ro) {  
  
  N = net_neurons$l[[1]]$len
  
  gr_it = 1
  #maxw_len = 0
  #invisible(sapply(1:net_neurons$l[[ro$learn_layer_id]]$len, function(n_id) maxw_len<<-max(maxw_len, length(net_neurons$l[[ro$learn_layer_id]]$weights[[n_id]]))))
  #gradients = array(0, dim=c(maxw_len, net_neurons$l[[ro$learn_layer_id]]$len, ro$Tmax %/% ro$learn_window_size)) 
  
  gradients = list()

  if(ro$collect_stat) {
    uum = NULL # for stat collecting
    ppm = NULL
  }

  sim_times = seq(ro$T0, ro$Tmax, by=ro$learn_window_size)
  for(sim_time0 in sim_times) {
    if(sim_time0<ro$Tmax) {
        sim_options = list(T0=sim_time0, Tmax= (sim_time0+ro$learn_window_size), dt=ro$dt, saveStat=ro$collect_stat)
        sim_out = net_neurons$sim(sim_options, net)
        
        if (ro$mode == "learn") {
          gradients[[length(gradients)+1]] = net_neurons$l[[ro$learn_layer_id]]$grad(sim_time0, sim_time0+ro$learn_window_size, net, ro$target_set)
        }     
    } else {
        break
    }
  }
  if(ro$mode == "learn") {
    lg = length(gradients) 
    if(lg>1) {
        gradients = lapply(2:lg, function(gi) mapply("+", gradients[[gi-1]], gradients[[gi]], SIMPLIFY=FALSE))
    }
    gradients = gradients[[1]]
    
    rew = reward_func(net[ net_neurons$l[[ro$learn_layer_id]]$ids ], ro)           
    gradients = lapply(gradients, function(sp) sp * rew)
    
    for(i in 1:net_neurons$l[[ro$learn_layer_id]]$len) {
        acc = ro$learning_rate * gradients[[i]]  - ro$weight_decay * net_neurons$l[[ro$learn_layer_id]]$weights[[i]]
        
        net_neurons$l[[ro$learn_layer_id]]$weights[[i]] =  net_neurons$l[[ro$learn_layer_id]]$weights[[i]] + acc 
    }
  }     
  
  if(ro$collect_stat) {
    return(list(net, net_neurons, sim_out$stat, grad=gradients ))
  } else {
    return(list(net, net_neurons))
  }
}
