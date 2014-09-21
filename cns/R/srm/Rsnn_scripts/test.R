#!/usr/bin/env RScript


require(Rsnn)

M = 100
const_ini = "/home/alexeyche/prog/alexeyche-junk/cns/c/snn_sim/constants.ini"
N = 100
jobs = 8
statLevel = 0

const = RConstants$new(const_ini)
const$setValue("sim","M", M)
const$setValue("layer","N", N)
const$setValue("layer","neuron_type", "WtaLayer")
const$setValue("layer","learning_rule", "SimpleSTDP")
const$setValue("layer","prob_fun", "Exp")
const$setValue("layer","lrate", 0.001)
const$setValue("layer","weight_decay_factor", 0.015)
const$setValue("layer","net_edge_prob", 1.0)
s = RSim$new(const, statLevel, jobs)

spikes = getSpikesFromMatrix(loadMatrix("/home/alexeyche/prog/sim/spikes/ucr/1_train_spikes",1))

# spikes=blank_net(M)
# spikes[[1]] = c(10,20)
# spikes[[2]] = c(20,30)
max_ep = 
for(i in 1:max_ep) {
    s$setInputSpikes(spikes)
    out_sp = s$run()
    cat(i,"\n")
}

stat = s$getLayerStat(0)

prast(out_sp,T0=0,Tmax=1000)

gr_pl(s$W()[[1]])