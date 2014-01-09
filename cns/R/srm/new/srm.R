
run_srm <- function(neurons, net, run_options) {
  attach(run_options, warn.conflicts=FALSE)
  
  neuron_map = sapply(neurons, function(n) n$get_id())
  N = length(neurons)
  learn_neurons_ids = NULL
  for(i in 1:N) {
    if(neuron_map[i] %in% learn_neurons) {
      learn_neurons_ids = c(learn_neurons_ids, i)
    }
  }
  if(collect_stat) {
    uum = NULL # for stat collecting
    ppm = NULL
    gr_it = 1
    maxw_len = 0
    invisible(sapply(learn_neurons_ids, function(n_id) maxw_len<<-max(maxw_len, length(neurons[[n_id]]$w))))
    gradients = array(0, dim=c(maxw_len, length(learn_neurons_ids), Tmax %/% learn_window_size))
  }
  
  T = seq(T0, Tmax, by=dt)
  for(t in T) {
    uu = sapply(neurons, function(n) n$u(t, net))
    pp = g(uu)
    fired = pp*dt>runif(N)
    idf = neuron_map[fired]
    if(NA %in% idf) {
      print(neurons[[11]]$w)
      print(neurons[[12]]$w)
    }
    #cat("pp" = pp, " idf=", idf, " uu=", uu, " fired=",fired, "\n", sep="")
    for(id in idf) {
      net[[id]] <- c(net[[id]], t)
      #cat("t: ", t, " spike of ", id, "\n")
    }
    if((mode == "learn")&&(t>0)&&(t %% learn_window_size == 0)) {
      gr = grad(neurons[learn_neurons_ids], t-learn_window_size, t, net, class, target_set) 
      invisible(sapply(1:length(learn_neurons_ids), function(i) neurons[[ learn_neurons_ids[i] ]]$w <- neurons[[ learn_neurons_ids[i] ]]$w + learning_rate * gr[[i]] ))
      if(collect_stat) {
        gradients[,,gr_it] = sapply(gr, function(row) { c(row, rep(0, maxw_len-length(row)))} )
        gr_it = gr_it + 1
      }
    }
    if(collect_stat) {
      ppm = rbind(ppm, pp) 
      uum = rbind(uum, uu)
    } 
  }
  if(collect_stat) {
    return(list(net, neurons, ppm, uum, grad=apply(gradients, c(1,2), mean)))
  } else {
    return(list(net, neurons))
  }
}
