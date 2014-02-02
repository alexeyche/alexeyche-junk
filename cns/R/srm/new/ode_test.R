#!/usr/bin/RScript

require(deSolve)

e0 <- 1.3 # mV
ts <- 3 # ms
tm <- 10 # ms

tf <- c(20, 27, 34, 41)
v=0.1

if_neuron = function(t, y, p) {
    ff = t-tf
    if(any(ff>0)) {
      t = min(ff[ff>0])
      du = e0*(-exp(-(t)/tm)/tm + exp(-(t)/ts)/ts)
      if(du<0) {
        du = -y[1]/tm
      }
      
    } else {
      du=0
    }  
    
    
    
    return(list( c(du) ) )
}

params = list(e0 = e0, ts = ts, tm = tm)

T0 = 0
Tmax = 150
dt = 0.5

out = ode(c(0), seq(T0, Tmax, by=dt), if_neuron, params, "rk4")
plot(out[,2], type="l")
