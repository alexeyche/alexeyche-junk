


setwd("~/my/git/alexeyche-junk/cns/R/srm")
dir= "/home/alexeyche/my/sim/runs/normal_case"
data_dir = "~/my/sim"

library(snn)
source('serialize_to_bin.R')

const_file = sprintf("%s/constants.R", dir)
source(const_file)
make_dataset_file = sprintf("%s/make_dataset.R", dir)
source(make_dataset_file)

source('gen_spikes.R')


source('eval_funcs.R')
source('plot_funcs.R')

constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 weight_per_neuron=weight_per_neuron, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)



model_file = sprintf("%s/model", dir)
ID_MAX=0
gr1 = TSNeurons(M)
neurons = SRMLayerClass$new(N, 0, 0)

loadModelFromFile(neurons, model_file)

s = SIMClass$new(list(neurons))
constants$tr=-45
s$sim(eval_sim_opt, constants, train_net_ev)
s$sim(eval_sim_opt, constants, test_net_ev)
train_resp = train_net_ev$getResponces()
test_resp = test_net_ev$getResponces()

plot_rastl(train_resp[[1]]$data)

