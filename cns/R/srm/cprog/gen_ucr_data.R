#!/usr/bin/RScript

library(snn)

source('../ucr_ts.R')
source('../gen_spikes.R')
source('../serialize_to_bin.R')
source('../plot_funcs.R')


ucr_spikes_dir = "/home/alexeyche/prog/sim/ucr_spikes"
data_dir = '~/prog/sim'
#data_dir = '~/my/sim'


N = 100
M = 100
mean_p_dur = 60000

duration = 1000
dt = 1
duration = 1000
samples_from_dataset = 20
epochs = 100

neurons = SRMLayerClass$new(N, 0, 0)


source('../make_dataset.R')

for(ep in 1:epochs) {
    patterns = patterns[sample(1:length(patterns))]
    
    #train_net_ev = NetClass$new(patterns, list(neurons), duration)
    #test_net_ev = NetClass$new(test_patterns, list(neurons), duration)
    
    #train_net_mean_p = NetClass$new(patterns, list(neurons), duration)
    #train_net_mean_p$replicate(mean_p_dur)
    
    train_net = NetClass$new(patterns, list(neurons), duration)
    train_net$replicate(train_net_ev$Tmax*1)
    
    train_net = NetClass$new(patterns, list(neurons), duration)
    spike_file = sprintf("%s/%s_ucr_%selems_%sclasses_%sdur", 
                         ucr_spikes_dir, 
                         ep, 
                         samples_from_dataset,
                         length(unique(train_net$labels)), 
                         duration)
    saveMatrixList(spike_file, list(list_to_matrix(train_net$net)))
}
