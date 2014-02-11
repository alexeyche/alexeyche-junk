
verbose = TRUE
dir='~/prog/sim/runs/test'
#dir='~/my/sim/runs/test'
data_dir = '~/prog/sim'
#data_dir = '~/my/sim'
setwd("~/prog/alexeyche-junk/cns/R/srm")
#setwd("~/my/git/alexeyche-junk/cns/R/srm")
source('constants.R')
source('srm_funcs.R')



require(snn)
set.seed(1234)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, ta=ta, sim_dim=sim_dim)


source('util.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('neuron.R')


ID_MAX=0
T0=0
Tmax=1000
dt=0.5
N=10
M=100
start_w.N = 0.25
start_w.M = 0.25

gr1 = TSNeurons(M = M)
neurons = SRMLayer(N, start_w.N, p_edge_prob=1)
connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )

net = list()

gr1$loadPatternFromFile("~/prog/sim/stimuli/sd1.csv", 100, 1, 0.5)
pattern = gr1$patterns[[1]]$data

net[gr1$ids] = pattern
net[neurons$ids] = -Inf



