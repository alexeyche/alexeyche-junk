source('../serialize_to_bin.R')
source('../util.R')
source('filters.R')
source('encode.R')
source('fb.R')
source('../ucr_ts.R')

nf = 100
fmin = 0
fmax = 400
Tdata = 50

c(trds, testds) := read_ts_file(synth, "~/prog/sim")

dir2save = "/home/alexeyche/prog/sim/ucr_fb_spikes"
it = 1
for(ds in trds) {
  nmax=550
  approx_data = rep(NA, nmax)                                      
  dt_d = length(ds$data)/nmax
  curt=0
  for(ri in 1:length(ds$data)) {
    curt = curt + nmax/length(ds$data)
    ct = ceiling(signif(curt, digits=5))                                        
    approx_data[ct] = ds$data[ri]
  }
  
  u = na.approx(approx_data)
  
  Fs = length(u)/Tdata
  net = encode_signal(u, nf, fmin, fmax, Fs)
  
  saveMatrixList(sprintf("%s/train_%s_cl%s", dir2save, it, ds$label), list( list_to_matrix(net) ) )
  it = it + 1
}

it = 1
for(ds in testds) {
  nmax=550
  approx_data = rep(NA, nmax)                                      
  dt_d = length(ds$data)/nmax
  curt=0
  for(ri in 1:length(ds$data)) {
    curt = curt + nmax/length(ds$data)
    ct = ceiling(signif(curt, digits=5))                                        
    approx_data[ct] = ds$data[ri]
  }
  
  u = na.approx(approx_data)
  
  Fs = length(u)/Tdata
  net = encode_signal(u, nf, fmin, fmax, Fs)
  
  saveMatrixList(sprintf("%s/test_%s_cl%s", dir2save, it, ds$label), list( list_to_matrix(net) ) )
  it = it + 1
}