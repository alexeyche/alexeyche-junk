#!/usr/bin/RScript

library(snn)

source('../ucr_ts.R')
source('../gen_spikes.R')
source('../serialize_to_bin.R')
source('../plot_funcs.R')

N = 100
M = 100
mean_p_dur = 60000
samples_from_dataset = 10
duration = 1000
dt = 1
spike_file = "/home/alexeyche/my/sim/input_spikes"

neurons = SRMLayerClass$new(N, 0, 0)

#data_dir = '~/prog/sim'
data_dir = '~/my/sim'

source('../make_dataset.R')

train_net = NetClass$new(patterns[1], list(neurons), 1000)

saveMatrixList(spike_file, list(list_to_matrix(patterns[[1]]$data)))
