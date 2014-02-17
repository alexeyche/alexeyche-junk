
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
set.seed(seed_num)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, added_lrate = added_lrate)

source('util.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('neuron.R')

ID_MAX=0
T0=0
Tmax=200
dt=1


N=1
M=2

net = list()


start_w.N = 2
start_w.M = 4

gr1 = TSNeurons(M = M)
neurons = SRMLayer(N, start_w.N, p_edge_prob=1)
connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)

neurons$connectFF(connection, start_w.M, 1:N )
sl = SimLayers(list(neurons))

net = list()

n1.dwdt = n2.dwdt = NULL
#neurons$mean_acc = 494
#neurons$mean_count = 1000

learn_window = 2
Ts = seq(T0,Tmax,length.out=200)
for(tr in 1:1) {
  uu = pp = grgr = NULL
  net[[1]] = c(50)
  net[[2]] = c(90)
  net[[3]] = c(-Inf)
  sim_opt = list(T0=0, Tmax=Tmax, dt=dt, saveStat=TRUE, seed=seed_num, learn=FALSE)
  s = sl$sim(sim_opt, net)
}

plot(Ts, s$stat$Cstat[1,1, ], type="l")
plot(Ts, s$stat$Cstat[1,2, ], type="l")
plot(Ts, s$stat$Bstat[1, ], type="l")
plot(Ts, s$stat$dwstat[1,1, ], type="l")
plot(Ts, s$stat$dwstat[1,2, ], type="l")
plot(Ts, s$stat$wstat[1,2, ], type="l")
plot(Ts, s$stat$wstat[1,1, ], type="l")
plot(Ts, s$stat$pstat[1, ], type="l")



