
verbose = TRUE
dir='~/prog/sim/runs/test'
#dir='~/my/sim/runs/test'
data_dir = '~/prog/sim'
#data_dir = '~/my/sim'
setwd("~/prog/alexeyche-junk/cns/R/srm")
#setwd("~/my/git/alexeyche-junk/cns/R/srm")
source('constants.R')
source('srm_funcs.R')

generatePoiss = function(M, lambda, T0, Tmax, dt) {
  net = vector("list", M)
  for(curt in seq(T0, Tmax, by=dt)) {
    fired = lambda*dt > runif(M)
    for(fi in which(fired==TRUE)) {
      net[[fi]] = c(net[[fi]], curt)
    }
  }
  return(net)
}


require(snn)
set.seed(seed_num)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, mean_time=mean_time, added_lrate = added_lrate)


source('util.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('neuron.R')
source('serialize_to_bin.R')

ID_MAX=0
T0=0
Tmax=10000
dt=1
N=1
M=100
start_w.N = 0.1
start_w.M = 0.3



gr1 = TSNeurons(M = M)

neurons = SRMLayer(N, start_w.N, p_edge_prob=0)
connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )
sl = SimLayers(list(neurons))

corr = 0.2
lambda = 10
net = list()
net[gr1$ids] = -Inf
net[neurons$ids] = -Inf
m = lambda
Mcorr = 0.2*M

Pxy_acc = Py_acc = I_acc = list()
wep2ep = NULL
for(ep in 101:200) {
  net = list()
  net[1:Mcorr] = generateCorrPoiss(Mcorr, lambda, corr, 0, Tmax, dt, 0.5)
  net[(Mcorr+1):M] = generatePoiss(M-Mcorr, lambda, 0, Tmax, dt)
  net[neurons$ids] = -Inf
  sim_opt = list(T0=0, Tmax=Tmax, dt=dt, saveStat=TRUE, seed=seed_num, learn=TRUE)
  s = sl$sim(sim_opt, net)
  Pxy_acc[[ep]] = s$stat$Pxy
  Py_acc[[ep]] = s$stat$Py
  I_acc[[ep]] = log(Pxy_acc[[ep]]/Py_acc[[ep]])
  wep2ep = rbind(wep2ep, neurons$weights[[1]])
  cat("ep: ", ep, "\n")
}

levelplot(wep2ep, col.regions=colorRampPalette(c("black", "white")))

