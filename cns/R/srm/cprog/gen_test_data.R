#!/usr/bin/RScript

library(snn)
setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")
#setwd("~/my/git/alexeyche-junk/cns/R/srm/cprog")

source('../ucr_ts.R')
source('../gen_spikes.R')
source('../serialize_to_bin.R')
source('../plot_funcs.R')


spike_file = "/home/alexeyche/prog/sim/test_spikes"

net=blank_net(10)

net[[1]] = c(1, 5, 10)
net[[5]] = c(4,8)
net[[10]] =c(2)

saveMatrixList(spike_file, list(list_to_matrix(net), 
                                matrix(c(100)),
                                matrix(c(1))
)
)

sp = loadMatrix("~/prog/sim/output_spikes",1)
neto = getSpikesFromMatrix(sp)
plot_rastl(neto,T0=0,Tmax=1000)

sf="~/prog/sim/test_stat"
p =loadMatrix(sf, 1)
u =loadMatrix(sf, 2)
B =loadMatrix(sf, 3)
W1 = loadMatrix(sf, 4)
C1 = loadMatrix(sf, 3+10+1)

plotl(p[1,])

plotl(u[1,])
plotl(B[1,])
plotl(C1[5,])
plotl(W1[5,])



mf ="~/prog/sim/test_model"
W = loadMatrix(mf,1)