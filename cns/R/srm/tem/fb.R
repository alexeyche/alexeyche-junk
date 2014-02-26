

encode_signal = function(u, nf, fmin, fmax, Fs) {
  dt_f = 1/Fs
  Ns = 2^9
  dt = dt_f/Ns
  Ft=1/dt
  flen = ceiling(0.03/dt)
  
  c(h, fc, t, f) := gammatone(nf, length(u), fmin, fmax, Ft)
  conv_s = matrix(0, nrow(h), ncol(h) )
  for(i in 1:nf) {
    conv_s[i,] = convolve(h[i,], u)
  }
  #gr_pl(t(conv_s))
  b = logspace(log10(1.3), log10(2.5), nf)
  d = 0.1 + runif(nf)
  kd = rep(0.1, nf)
  net = list()
  for(i in 1:nf) {
    net[[i]] = iaf_encode(conv_s[i, ], dt, b[i], d[i], 0, Inf, kd[i])
  }
  return(net)
}


