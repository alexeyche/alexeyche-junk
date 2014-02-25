

require(zoo)
require(wavethresh)

source('../util.R')
source('../plot_funcs.R')
source('../snn/R/util.R')

source('../ucr_ts.R')


c(trds, testds) := read_ts_file(synth, "~/prog/sim")

el= 55
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

Tdata = 50 # sec
Fs = length(u)/Tdata
net = encode_signal(u, 100, 0, 400, Fs)
plot_rastl(net)

