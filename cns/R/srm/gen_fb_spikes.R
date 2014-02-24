#!/usr/bin/RScript

library(snn)

source('plot_funcs.R')

constants = list(tau = 1, sim_dim=1000)

N = 1500
HiHerz = 0
LowHerz = 2000
gain = 500 # Hz
fstim = 200 # Hz

Tmax=10000
T0 = 0
dt = 1

Ts = seq(T0, Tmax, length.out = (Tmax-T0)/dt)

fbl = FBLayerClass$new(N, HiHerz, LowHerz, constants)
fbl$setInput( gain*(sin(4*pi*fstim*Ts) + sin(6*pi*fstim*Ts))/constants$sim_dim)

net = blank_net(N)

sm = SIMClass(list(fbl))

sim_opt = list(T0=T0, Tmax=Tmax, dt=dt, saveStat = TRUE, learn=FALSE)

sm$sim(sim_opt, constants, net)
