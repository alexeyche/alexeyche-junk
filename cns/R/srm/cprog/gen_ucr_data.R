#!/usr/bin/RScript

library(snn)
setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")
#setwd("~/my/git/alexeyche-junk/cns/R/srm/cprog")

source('../ucr_ts.R')
source('../gen_spikes.R')
source('../serialize_to_bin.R')
source('../plot_funcs.R')


ucr_spikes_dir = "/home/alexeyche/prog/sim/ucr_spikes"
#ucr_spikes_dir = "/home/alexeyche/my/sim/ucr_spikes"
-data_dir = '~/prog/sim'
#data_dir = '~/my/sim'


M = 100
mean_p_dur = 60000

duration = 1000
dt = 1
duration = 1000
samples_from_dataset = 20
epochs = 100

source('../make_dataset.R')

for(ep in 1:epochs) {
    patterns = patterns[sample(1:length(patterns))]
    
    train_net = NetClass$new(patterns, duration)
    
    dir.create(sprintf("%s/train", ucr_spikes_dir))
    spike_file = sprintf("%s/train/%s_ucr_%selems_%sclasses_%sdur", 
                         ucr_spikes_dir, 
                         ep, 
                         samples_from_dataset,
                         length(unique(train_net$labels)), 
                         duration)
    saveMatrixList(spike_file, list(list_to_matrix(train_net$net), matrix(train_net$timeline), matrix(train_net$labels) ))
}

test_net = NetClass$new(test_patterns, duration)
dir.create(sprintf("%s/test", ucr_spikes_dir))
spike_file = sprintf("%s/test/ucr_%selems_%sclasses_%sdur", 
                     ucr_spikes_dir, 
                     samples_from_dataset,
                     length(unique(train_net$labels)), 
                     duration)
saveMatrixList(spike_file, list(list_to_matrix(test_net$net), 
                                matrix(test_net$timeline),
                                matrix(test_net$labels)
                                )
               )
