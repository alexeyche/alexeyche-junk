

setwd("~/my/git/alexeyche-junk/cns/R/srm")
library(snn)
source('serialize_to_bin.R')

dir= "/home/alexeyche/my/sim/runs/test"
const_file = sprintf("%s/constants.R", dir)
source(const_file)

source('gen_spikes.R')
data_dir = "~/my/sim"
source('make_dataset.R')
source('eval_funcs.R')
source('plot_funcs.R')

set.seed(seed_num)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, ws4=ws^4, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)



model_file = sprintf("%s/model", dir)
ID_MAX=0
gr1 = TSNeurons(M)
neurons = SRMLayerClass$new(N, 0, 0)

loadModelFromFile(neurons, model_file)

s = SIMClass$new(list(neurons))

responces = eval_patterns(patterns, gr1, neurons, s, duration)

plot_rastl(responces[[1]]$data)

