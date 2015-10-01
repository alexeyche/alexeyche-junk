#!/usr/bin/env Rscript
tau_rise = 10.0001
tau_decay = 10
epsp = Vectorize(function(s) 1/(1-tau_rise/tau_decay) * (exp(-s/tau_decay) - exp(-s/tau_rise)), "s" )

t = seq(0,1000)
s = t
o = epsp(t)
