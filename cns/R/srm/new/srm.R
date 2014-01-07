
run_srm <- function(neurons, net, run_options) {
  neuron_map = sapply(neurons, function(n) n$get_id())
  attach(run_options, warn.conflicts=FALSE)
  N = length(neurons)
  
  if(collect_stat) {
    uum = NULL # for stat collecting
    ppm = NULL
    gr_it = 1
    gradients = array(0, dim=c(length(neurons[[1]]$w), N, Tmax %/% learn_window_size))
  }
  
  T = seq(T0, Tmax, by=dt)
  for(t in T) {
    uu = sapply(neurons, function(n) n$u(t, net))
    pp = g(uu)
    fired = pp*dt>runif(N)
    idf = neuron_map[fired]
    for(id in idf) {
      net[[id]] <- c(net[[id]], t)
      #cat("t: ", t, " spike of ", id, "\n")
    }
    if((mode == "learn")&&(t>0)&&(t %% learn_window_size == 0)) {
      gr = grad(neurons, t-learn_window_size, t+learn_window_size, net) 
      invisible(sapply(1:N, function(i) neurons[[i]]$w <- neurons[[i]]$w + learning_rate * gr[,i] ))
      if(collect_stat) {
        gradients[,,gr_it] = gr
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
