#!/usr/bin/RScript

require(deSolve)

e0 <- 1.3 # mV
ts <- 3 # ms
tm <- 10 # ms

if_neuron = function(t, y, p) {
    dv = y[1] + y[2]*(y[1] - 0)
    dres = - y[2]/10
    return(list( c(dv, dres) ))
}

params = list(e0 = e0, ts = ts, tm = tm)

T0 = 0
Tmax = 150
dt = 0.5

out = ode(c(-10, 1), seq(T0, Tmax, by=dt), if_neuron, params, "rk4")
plot(out[,2], type="l")
