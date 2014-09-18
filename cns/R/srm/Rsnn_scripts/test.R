#!/usr/bin/env RScript


require(Rsnn)

const_ini = "/home/alexeyche/prog/alexeyche-junk/cns/c/snn_sim/constants.ini"
const = RConstants$new(const_ini)
const$setValue("sim","M", 10)
const$setValue("layer","N", 10)

s = RSim$new(const, 0, 8)

spikes = blank_net(10)
spikes[[1]] = c(10, 20)
spikes[[2]] = c(2, 5)

s$setInputSpikes(spikes)
