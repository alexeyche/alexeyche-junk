setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
dir = '~/my/sim'

source('util.R')
source('plot_funcs.R')
source('ucr_ts.R')
source('gen_spikes.R')

data = synth # synthetic control
c(train, test) := read_ts_file(data)


duration = 300

N = 10
start_w = 5
M = 20
dt = 0.5

gr1 = TSNeurons(M = M)
gr1$loadPatterns(train, duration, dt, lambda=4)
#plot_rastl(gr1$patterns[[3]]$data)

neurons = SRMLayer(N, start_w/4)
neurons$connectFF(gr1$ids, start_w)
