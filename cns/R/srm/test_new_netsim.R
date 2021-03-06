
library(snn)
setwd("/home/alexeyche/prog/alexeyche-junk/cns/R/srm")
source('constants.R')
source('plot_funcs.R')
duration = 1000
#data_dir = '~/my/sim'
data_dir = '~/prog/sim'

constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 weight_per_neuron=weight_per_neuron, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)

ID_MAX=0
gr1 = TSNeurons(M = M)
N = 100
neurons = SRMLayerClass$new(N, 0, p_edge_prob=net_edge_prob,ninh=round(N*inhib_frac), syn_delay_rate, axon_delay_rate, delay_dist_gain)

source('make_dataset.R')

connection = matrix(gr1$ids(), nrow=length(gr1$ids()), ncol=N)
for(i in 1:N) {
  cc = sample(gr1$ids(), M-N/10)
  connection[cc,i] = 0
}

neurons$connectFF(connection, 0, 1:N, syn_delay_rate, delay_dist_gain)
for(ni in 1:N) {
    ncon = length(neurons$obj$id_conns[[ni]])
    neurons$obj$W[[ni]] = abs(rnorm(ncon, mean=weight_per_neuron/ncon, sd=start_W.sd))
}

sl = list(neurons)
s = SIMClass$new(sl)

train_net_ev = NetClass$new(patterns, list(neurons), duration)

sim_opt = list(dt=dt, saveStat=FALSE, learn=TRUE, determ=FALSE)
s$sim(sim_opt, constants, train_net_ev)

plot_rastl(train_net_ev$net)
