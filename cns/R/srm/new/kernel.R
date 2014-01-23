
SQRT_2PI = sqrt(2*pi)


gaussian_kernel = function(s, sigma) {
  sum((1/SQRT_2PI)*exp(-(s^2)/sigma))
}

get_finger_print = function(spikes, T0, Tmax, window, sigma) {
  K = matrix(0, nrow=length(spikes), ncol=(Tmax-T0)/window)
  for(sp_i in 1:length(spikes)) {
    if(length(spikes[[sp_i]]) ==0) next
    for(t0_i in 1:((Tmax-T0)/window)) {
      K[sp_i, t0_i] = integrate( Vectorize(function(t) gaussian_kernel(t-spikes[[sp_i]], sigma)), (t0_i-1)*window, (t0_i-1)*window + window)$value 
    }
  }
  return(K)
}


x = lapply(net_all, function(x) get_finger_print(x$data, 0, 300, 30, 200))
x = simplify2array(x)
xm = apply(x, c(1,2), mean)
x1 = x[,,1]

  




