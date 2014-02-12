
apply_grad_norm <- function(weights, grads, ro) {
    if(ro$weights_norm_type == 'mult_glob') {
        w = unlist(weights)
        dw = unlist(grads)
        new_weights = lapply(1:length(weights), function(i) {
            sqrt(sum(w^2))*(weights[[i]] + ro$learning_rate*grads[[i]])/sqrt(sum( (w+ro$learning_rate*dw)^2 ))
        })
        return(new_weights)
    } else 
    if(ro$weights_norm_type == 'mult_local') {
        new_weights = lapply(1:length(weights), function(i) {
            dw = ro$learning_rate*grads[[i]]       
            sqrt(sum(weights[[i]]^2))*(weights[[i]]+dw)/sqrt( sum( (weights[[i]] + dw)^2))
        })
        return(new_weights)
    } else 
    if(ro$weights_norm_type == 'add') {
        mw = mean(unlist(grads))       
        new_weights = lapply(1:length(weights), function(i) weights[[i]] + ro$learning_rate*(grads[[i]]-mw))
        return(new_weights)
    } else
    if(ro$weights_norm_type == 'no') {
        new_weights = lapply(1:length(weights), function(i) weights[[i]] + ro$learning_rate*grads[[i]])
        return(new_weights)         
    } else {
        cat(sprintf("Can't find code for weight normalization type %s\n", ro$weights_norm_type))
        q()
    }
}

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
    grad_stat = NULL
  }
  
       
  
  if(ro$collect_stat) {
    return(list(net, net_neurons, sim_out$stat, grad=list(gradients, mean(grad_stat), mean(rew)) ))
  } else {
    return(list(net, net_neurons))
  }
}
