#!/usr/bin/RScript

library(snn)

source('../ucr_ts.R')
source('../gen_spikes.R')
source('../serialize_to_bin.R')
source('../plot_funcs.R')

N = 1
M = 2
mean_p_dur = 60000
samples_from_dataset = 10
duration = 1000
dt = 1
#spike_file = "/home/alexeyche/my/sim/input_spikes"
spike_file = "/home/alexeyche/prog/sim/input_spikes"

neurons = SRMLayerClass$new(N, 0, 0)

#data_dir = '~/prog/sim'
data_dir = '~/my/sim'

#source('../make_dataset.R')
p = list()
p[[1]] = c(1,6,11,16)
p[[2]] = c(2,7,12,17)+1
p[[3]] = c(3,8,13,18)+2
p[[4]] = c(4,9,14,19)+3
p[[5]] = c(5,10,15,20)+4

patterns = list(list(data=p, label=1))
train_net = NetClass$new(patterns[1], list(neurons), 1000)

saveMatrixList(spike_file, list(list_to_matrix(patterns[[1]]$data)))
