

gaussian_kernel = function(s, sigma) {
  sum((1/sqrt(2*pi*sigma^2))*exp(-(s^2)/(2*sigma^2)))
}

get_finger_print = function(spikes, T0, Tmax, window, sigma) {
  K = matrix(0, nrow=length(spikes), ncol=(Tmax-T0)/window)
  scale_factor = 1
  for(sp_i in 1:length(spikes)) {
    if(length(spikes[[sp_i]]) == 0) next
    if((length(spikes[[sp_i]]) == 1)&&(spikes[[sp_i]][1] == -Inf)) next
    for(t0_i in 1:((Tmax-T0)/window)) {
      K[sp_i, t0_i] = integrate( Vectorize(function(t) gaussian_kernel(t-spikes[[sp_i]], sigma)), (t0_i-1)*window, (t0_i-1)*window + window)$value 
    }
    scale_factor = scale_factor + 1
  }
  return(K/scale_factor)
}

get_mean_activity = function(net_all, ro) {
  x = lapply(net_all, function(x) get_finger_print(x$data, ro$T0, ro$Tmax, ro$fp_window, ro$fp_kernel_size))
  x = simplify2array(x)
  xm = apply(x, c(1,2), mean)
  dd = sapply(1:dim(x)[3], function(i) sum((xm - x[,,i])^2))  
  return(list(mean_act=xm, deviation_range=c(min(dd), max(dd))) )
}

reward_func = function(curr_act, ro) {
  if(!is.null(ro$mean_activity_stat)) {
      fp = get_finger_print(curr_act, ro$T0, ro$Tmax, ro$fp_window, ro$fp_kernel_size)
      c(mean_act, dev_range) := ro$mean_activity_stat
      dev_norm = ( sum( (fp-mean_act)^2)-dev_range[1])/(dev_range[2]-dev_range[1])
      return(dev_norm - ro$dev_frac_norm)  
  }
  return(1.0)
}



