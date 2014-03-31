#!/usr/bin/RScript
setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")
source('../serialize_to_bin.R')
source('../util.R')
source('../plot_funcs.R')

library(snn)
workdir="~/prog/sim/cprog/runs/rfd"


input_file = "/home/alexeyche/prog/sim/rfd_files/ep_1_30.0sec"
ep=3
model_file = sprintf("%s/%d_model", workdir, ep)
stat_file = sprintf("%s/%d_stat", workdir, ep)
output_spikes = sprintf("%s/%d_output_spikes", workdir, ep)

#sp = loadMatrix(input_file,1)
sp = loadMatrix(output_spikes,1)
net = blank_net(nrow(sp))
for(i in 1:length(net)) {
    net[[i]] = sp[i, sp[i,]>0]
}
plot_rastl(net,T0=0,Tmax=1000)

if(file.exists(sprintf("%s.bin",stat_file))) {
    p = loadMatrix(stat_file, 1)
    u = loadMatrix(stat_file, 2)
    B = loadMatrix(stat_file, 3)
    w1 = loadMatrix(stat_file, 3+1)
    C1 = loadMatrix(stat_file, 3+10+1)
    syn=80
    par(mfrow=c(2,1))
    spikes = net[[101]][net[[101]]<1000]
    plot(spikes, rep(1,length(spikes)), xlim=c(0,1000) )
    #plotl(C1[syn,1:1000])
    plotl(w1[syn,1:1000])
}

W = loadMatrix(model_file,1)

#gr_pl(t(W))

N=10
Wacc = vector("list",N)
pacc = vector("list",N)
for(ep in 3:200) {
    model_file = sprintf("%s/%d_model", workdir, ep)
    if(file.exists(sprintf("%s.bin",model_file))) {
        W = loadMatrix(model_file,1)
        pmean = loadMatrix(model_file,3)
        for(ni in 1:N) {
            Wacc[[ni]] = cbind(Wacc[[ni]], W[ni,])    
            pacc[[ni]] = c(pacc[[ni]], c(pmean[ni,]))
        }
        
    }
}

#gr_pl(t(Wacc[[1]]))