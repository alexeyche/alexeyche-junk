#!/usr/bin/env RScript

library(snn)

verbose = TRUE
#dir='~/prog/sim/runs/test'
dir='~/my/sim/runs/test'
#data_dir = '~/prog/sim'
data_dir = '~/my/sim'
#setwd("~/prog/alexeyche-junk/cns/R/srm")
setwd("~/my/git/alexeyche-junk/cns/R/srm")
source('constants.R')
source('srm_funcs.R')

constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, 
                 pr=pr, gain_factor=gain_factor, ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, added_lrate = added_lrate, sim_dim=sim_dim)

T0 = 0
Tmax = 200
dt = 1
M = 2
N = 1
start_w.N = 4
start_w.M = 4

gr1 = TSNeurons$new(M)
l = SRMLayerClass$new(N, start_w.N, net_edge_prob)
l$connectFF(gr1$ids, start_w.M)

s = SIMClass$new(list(l))


sim_opt = list(T0=T0, Tmax=Tmax, dt=dt, saveStat=TRUE, learn=TRUE)
net = list()
net[[1]] = 50
net[[2]] = 90
net[[3]] = numeric(0)

#invisible(sapply( (M+1):(M+N), function(id) net[[id]] <<- numeric(0) ))

s$sim(sim_opt, constants, net)

plotl(l$obj$stat_p[[1]])
Cm = list_to_matrix(l$obj$stat_C[[1]])
Wm = list_to_matrix(l$obj$stat_W[[1]])
plotl(Wm[,1])
plotl(Wm[,2])
plotl(Cm[,2])
plotl(l$obj$stat_B[[1]])
