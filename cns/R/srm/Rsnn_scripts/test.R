#!/usr/bin/env RScript


require(Rsnn)

const_ini = "/home/alexeyche/prog/alexeyche-junk/cns/c/snn_sim/constants.ini"
const = RConstants$new(const_ini)
s = RSim$new(const, 0, 8)
