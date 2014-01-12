
run_srm <- function(layers, net, run_options) {
  attach(run_options, warn.conflicts=FALSE)
  
  N = layers[[1]]$len
  
  if(collect_stat) {
    uum = NULL # for stat collecting
    ppm = NULL
    gr_it = 1
    maxw_len = 0
    invisible(sapply(1:layers[[learn_layer_id]]$len, function(n_id) maxw_len<<-max(maxw_len, length(layers[[learn_layer_id]]$weights[[n_id]]))))
    gradients = array(0, dim=c(maxw_len, layers[[learn_layer_id]]$len, Tmax %/% learn_window_size))
  }
  stat = list()
  for(i in 1:length(layers)) {
    stat[[i]] = list()
    stat[[i]]$p = NULL
    stat[[i]]$u = NULL
  }
  T = seq(T0, Tmax, by=dt)
  for(time in T) {
    i = 1
    for(neurons in layers) {
      if (!neurons$stochastic) next
      if(i==1) {
        uu = neurons$u(time,net)
        pp = g(uu)  
      } else {
        pp = neurons$rate(time,net)
        uu = NULL
      }
      
      fired = pp*dt>runif(neurons$len)
      idf = neurons$ids[fired]
      
      #cat("pp" = pp, " idf=", idf, " uu=", uu, " fired=",fired, "\n", sep="")
      for(id in idf) {
        net[[id]] <- c(net[[id]], t)
        #cat("t: ", t, " spike of ", id, "\n")
      }
      if(collect_stat) {
        stat[[i]]$p = rbind(stat[[i]]$p, pp) 
        stat[[i]]$u = rbind(stat[[i]]$u, c(uu))
      }
      i=i+1
    }
    if((mode == "learn")&&(t>0)&&(t %% learn_window_size == 0)) {
      target_set$class = run_options$class
      gr = layers[[learn_layer_id]]$grad(t-learn_window_size, t, net, target_set)
      
      invisible(sapply(1:layers[[learn_layer_id]]$len, function(i) layers[[learn_layer_id]]$weights[[i]] = layers[[learn_layer_id]]$weights[[i]] + learning_rate * gr[[i]] ))
      if(collect_stat) {
        gradients[,,gr_it] = sapply(gr, function(row) { c(row, rep(0, maxw_len-length(row)))} )
        gr_it = gr_it + 1
      }
    }
    
     
  }
  
  if(collect_stat) {
    return(list(net, layers, stat, grad=apply(gradients, c(1,2), mean)))
  } else {
    return(list(net, layers))
  }
}
