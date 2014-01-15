
run_srm <- function(layers, net, ro) {  
  
  N = layers[[1]]$len
  
  if(ro$collect_stat) {
    uum = NULL # for stat collecting
    ppm = NULL
    gr_it = 1
    maxw_len = 0
    invisible(sapply(1:layers[[ro$learn_layer_id]]$len, function(n_id) maxw_len<<-max(maxw_len, length(layers[[ro$learn_layer_id]]$weights[[n_id]]))))
    gradients = array(0, dim=c(maxw_len, layers[[ro$learn_layer_id]]$len, ro$Tmax %/% ro$learn_window_size))
  }
  stat = list()
  for(i in 1:length(layers)) {
    stat[[i]] = list()
    stat[[i]]$p = NULL
    stat[[i]]$u = NULL
  }
  T = seq(ro$T0, ro$Tmax, by=ro$dt)
  for(time in T) {
    i=1
    for(neurons in layers) {      
      uu = neurons$u(time, net)
      pp = g(uu)  
      
      fired = pp*ro$dt>runif(neurons$len)
      idf = neurons$ids[fired]
      
      #cat("pp" = pp, " idf=", idf, " uu=", uu, " fired=",fired, "\n", sep="")
      for(id in idf) {
        net[[id]] <- c(net[[id]], time)
        #cat("t: ", time, " spike of ", id, "\n")
      }
      if(ro$collect_stat) {
        stat[[i]]$p = rbind(stat[[i]]$p, pp) 
        stat[[i]]$u = rbind(stat[[i]]$u, c(uu))
      }
      i=i+1
    }
    if((ro$mode == "learn")&&(time>0)&&(time %% ro$learn_window_size == 0)) {      
      gr = layers[[ro$learn_layer_id]]$grad(time-ro$learn_window_size, time, net, ro$target_set)
      
      invisible(sapply(1:layers[[ro$learn_layer_id]]$len, function(i) layers[[ro$learn_layer_id]]$weights[[i]] = layers[[ro$learn_layer_id]]$weights[[i]] + ro$learning_rate * gr[[i]] ))
      if(ro$collect_stat) {
        gradients[,,gr_it] = sapply(gr, function(row) { c(row, rep(0, maxw_len-length(row)))} )
        gr_it = gr_it + 1
      }
    }
    
     
  }
  
  if(ro$collect_stat) {
    return(list(net, layers, stat, grad=apply(gradients, c(1,2), mean)))
  } else {
    return(list(net, layers))
  }
}
