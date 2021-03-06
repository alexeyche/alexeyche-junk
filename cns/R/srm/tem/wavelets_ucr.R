#!/usr/bin/RScript

source('../serialize_to_bin.R')
source('../util.R')
source('filters.R')
source('encode.R')
source('fb.R')
source('../ucr_ts.R')
source('tem_util.R')
source('../plot_funcs.R')

require(wavelets)
require(entropy)

c(trds, testds) := read_ts_file(synth, "~/prog/sim")
#dir2save = "/home/alexeyche/my/sim/ucr_fb_spikes/wavelets"
dir2save = "/home/alexeyche/prog/sim/ucr_fb_spikes/wavelets"

wave_all = list()
for(ds_and_label in list(list(trds, "train"), list(testds, "test"))) {
    ds = ds_and_label[[1]]
    label = ds_and_label[[2]]
    ds_mx = list()
    for(i in 1:length(ds)) {

        xapp = approx_ts(ds[[i]]$data, 520) # 520 ~ 512

        f = wt.filter('d4', modwt=TRUE)

        nf = as.integer(log2(length(xapp)))
        mx = modwt(xapp, f, nf)
        
        ds_mx[[i]] = list_to_matrix(mx@W)
        wave_all[[length(wave_all)+1]] = mx    
    }
#    saveMatrixList(sprintf("%s/%s_wavelets", dir2save, label), ds_mx)
}

dt = 0.1
t_rc = 2
t_ref = 0.2


sp_wave = list()
for(i in 1:nf) {
  u = wave_all[[55]]@W[[i]]
  Tmax = length(u)*dt
  
  rate_need = 0.3*1/t_ref
  #plot(function(r) 1/(1-exp( (t_ref-(1.0/r))/t_rc)), xlim=c(-10,10))
  z = 1/(1-exp( (t_ref-(1.0/rate_need))/t_rc ))
  
  intercept = 0
  g = (1 - z)/(intercept - 1.0)
  b = 1 - g*intercept
  
  sp_wave[[i]] = lif_encode(u*g+b, dt, t_rc, t_ref)
}
plot_rastl(sp_wave)
sp_wave[[i]] = lif_encode(u*g+0, dt, t_rc, t_ref)

par(mfrow=c(2,1))
plot(seq(0,Tmax,length.out =length(u)), u, xlim=c(0,Tmax), type="l")
plot(sp, rep(1,length(sp)), xlim=c(0,Tmax))

r = length(sp)/Tmax

  
nn = net[gr1$ids()]
nb = 300
bk = matrix(0, length(nn), nb)
for(i in 1:length(nn)) {
  h = hist(nn[[i]], breaks=nb, plot=FALSE)
  j = 1+h$breaks[1:(length(h$breaks)-1)]/(h$breaks[2]-h$breaks[1])
  bk[i, j] = h$counts
}
gr_pl(cov(t(bk)))



