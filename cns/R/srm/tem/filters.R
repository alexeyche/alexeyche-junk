gammatone = function(num, len, fmin, fmax, fs, pad_bw = 2) {
  EarQ = 9.26449
  minBW = 24.7
  order = 4
  dt = 1/fs
  t = dt*(0:(len-1))
  f = (0:(length(t)-1))/length(t)*fs
  beta = 1.019
  
  Wp = fmax
  fmax = EarQ*(Wp-pad_bw*beta*minBW)/(EarQ+pad_bw*beta)
  
  overlap = EarQ*(log(fmax+EarQ*minBW)-log(fmin+EarQ*minBW))/max(1,num-1)
  fc = -EarQ*minBW + (fmax+EarQ*minBW)*exp(-(num-(1:num))*overlap/EarQ)
  h = matrix(0, num, len) 
  for(i in 1:num) {
    h[i,] = t^(order-1) * exp(-2*pi*beta*(fc[i]/EarQ+minBW)*t) * cos(2*pi*fc[i]*t)
    h[i,] = h[i,]/max(abs(fft(h[i,])))
  }
  return(list(h, fc, t, f))
}
