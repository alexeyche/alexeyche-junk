#!/usr/bin/RScript

require(deSolve)

e0 <- 1.3 # mV
ts <- 3 # ms
tm <- 10 # ms

if_neuron = function(t, y, p) {
<<<<<<< HEAD
    dv = y[1] + y[2]*(y[1] - 0)
    dres = - y[2]/10
=======
    dv = (y[1])*(y[2] - 0.3) +1
    dres = - y[2]/(p$ts*2)
>>>>>>> e15fe16acaa261d17f6c05cee74939a49a5ff2da
    return(list( c(dv, dres) ))
}

params = list(e0 = e0, ts = ts, tm = tm)

T0 = 0
Tmax = 150
dt = 0.5

<<<<<<< HEAD
out = ode(c(-10, 1), seq(T0, Tmax, by=dt), if_neuron, params, "rk4")
plot(out[,2], type="l")
=======
out = ode(c(0, 1), seq(T0, Tmax, by=dt), if_neuron, params, "rk4")
plot(out[,2] ,type="l")
>>>>>>> e15fe16acaa261d17f6c05cee74939a49a5ff2da
