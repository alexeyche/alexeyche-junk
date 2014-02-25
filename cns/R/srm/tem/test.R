

require(zoo)
require(wavethresh)

source('../util.R')
source('../plot_funcs.R')
source('../snn/R/util.R')

source('../ucr_ts.R')

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

iaf_encode = function(u, dt, b, d, sd_tr, R, C) {
  y = 0
  interval = 0
  sp = NULL
  
  int_meth = 'trapz'
  last = length(u)
  if(int_meth == 'quad') {
    comp_y = function(y, i) y + dt*(b+u[i])/C
  } else
    if(int_meth == 'trapz') {
      comp_y = function(y, i) y + dt*(b+(u[i]+u[i+1])/2)/C
      last = last-1
    }
  
  dnew = d+rnorm(1, 0, sd_tr)
  for(i in 1:last) {
    y = comp_y(y, i)
    interval = interval + dt
    if(y>=dnew) {
      sp = c(sp, interval)
      y = y - dnew
      dnew = d+abs(rnorm(1, 0, sd_tr))
    }
  }
  return(sp)
}
c(trds, testds) := read_ts_file(synth, "~/my/sim")

el= 1
data = trds[[el]]$data

nmax=550
approx_data = rep(NA, nmax)                                      
dt_d = length(trds[[el]]$data)/nmax
curt=0
for(ri in 1:length(data)) {
  curt = curt + nmax/length(data)
  ct = ceiling(signif(curt, digits=5))                                        
  approx_data[ct] = data[ri]
}

u = na.approx(approx_data)

Tdata = 101 # sec
Fs = length(u)/Tdata
dt_f = 1/Fs
Ns = 2^9
dt = dt_f/Ns
Ft=1/dt
flen = ceiling(0.03/dt)
fmin=0
fmax=500
nf = 100

Ts = seq(0,1,length.out=length(u))
utr = u[1:500]
U = fft(utr)/length(u)
U= U[1:length(U)/2]
f = Fs/2*seq(0,1, length.out=length(U))
#plot(f, abs(U), type="l")

#ywd = wd(approx_data[1:512],filter.number=2, family='DaubExPhase', type='station')
#plot(ywd, scaling="by.level")

c(h, fc, t, f) := gammatone(nf, length(u), fmin, fmax, Ft)


conv_s = matrix(0, nrow(h), ncol(h) )

#data = c(sapply(trds[1:25], function(x) x$data))
#data = c(sapply(trds[51:75], function(x) x$data))
for(i in 1:nf) {
  conv_s[i,] = convolve(h[i,], u)
}
#filled.contour(t(conv_s))
gr_pl(t(conv_s))


b = logspace(log10(1.3), log10(2.5), nf)
d = 0.5 + runif(nf)
kd = rep(0.01, nf)
net = list()
for(i in 1:nf) {
  net[[i]] = iaf_encode(conv_s[i, ], 0.000025, b[i], d[i], 0, Inf, kd[i])
}
plot_rastl(net)
