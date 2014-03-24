
library(snn)
source('constants.R')
duration = 100
data_dir = '~/my/sim'

constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 weight_per_neuron=weight_per_neuron, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)

ID_MAX=0
gr1 = TSNeurons(M = M)
neurons = SRMLayerClass$new(N, 0, p_edge_prob=net_edge_prob,ninh=round(N*inhib_frac), syn_delay_rate, axon_delay_rate, delay_dist_gain)

source('make_dataset.R')

connection = matrix(gr1$ids(), nrow=length(gr1$ids()), ncol=N)
for(i in 1:net_neurons_for_input) {
  cc = sample(gr1$ids(), M-afferent_per_neuron)
  connection[cc,i] = 0
}

neurons$connectFF(connection, 0, 1:N, syn_delay_rate, delay_dist_gain)
sl = list(neurons)
s = SIMClass$new(sl)

train_net_ev = NetClass$new(patterns[1], list(neurons), duration)

sim_opt = list(dt=dt, saveStat=FALSE, learn=TRUE, determ=FALSE)
s$sim(sim_opt, constants, train_net_ev)

