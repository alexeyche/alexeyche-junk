#!/usr/bin/RScript
#setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")
setwd("~/my/git/alexeyche-junk/cns/R/srm/cprog")
source('../serialize_to_bin.R')
source('../util.R')

source('../plot_funcs.R')


library(snn)

#rundir="/home/alexeyche/prog/sim/runs"
rundir="/home/alexeyche/my/sim/runs"
#runname = "test_run"
#runname = "n50_no_conn"
#runname = "n50_conn_3"
#runname = "n50_conn_big"
runname = "n100_exp_no_conn"
workdir=sprintf("%s/%s", rundir, runname)

for(ep in 1:300) {
    output_spikes = sprintf("%s/%s_output_spikes.bin", workdir, ep)
    if(!file.exists(output_spikes)) { ep=ep-1; break }
}
#ep=2

#workdir="/home/alexeyche/prog/sim/runs/rfd"


ep_str=""
if(ep>0) {
    ep_str = sprintf("%d_",ep)
}
model_file = sprintf("%s/%smodel", workdir, ep_str)
stat_file = sprintf("%s/%sstat", workdir, ep_str)
output_spikes = sprintf("%s/%soutput_spikes", workdir, ep_str)
const_ini = sprintf("%s/constants.ini", workdir)


N = as.integer(get_const("N"))

#sp = loadMatrix(input_file,1)
sp = loadMatrix(output_spikes,1)
net = blank_net(nrow(sp))
for(i in 1:length(net)) {
    spike_elems = which(sp[i,]>0)
    if(length(spike_elems)>0) {
        if(sp[i,1] == 0) {
            spikes_elems = c(1, spike_elems)
        }
    }
    net[[i]] = sp[i, spike_elems]
}
p1 = plot_rastl(net,T0=0,Tmax=2000)

if(file.exists(sprintf("%s.bin",stat_file))) {
    p = loadMatrix(stat_file, 1)
    u = loadMatrix(stat_file, 2)
    B = loadMatrix(stat_file, 3)
    syn=75
    nid=2
    dWn = loadMatrix(stat_file, 3+nid)
    Cn = loadMatrix(stat_file, 3+N+nid)
#     par(mfrow=c(3,1))
#     spikes = net[[100+nid]][net[[100+nid]]<1000]
#     plot(spikes, rep(1,length(spikes)), xlim=c(0,1000) )
#     plotl(C1[syn,1:1000])
#     plotl(w1[syn,1:1000])
    #plotl(B[nid,1:1000])
}

W = loadMatrix(model_file,1)

p2 = levelplot(t(W), col.regions=colorRampPalette(c("black", "white")))

print(p1, position=c(0, 0.5, 1, 1), more=TRUE)
print(p2, position=c(0, 0, 1, 0.5))
# Wacc = vector("list",N)
# pacc = vector("list",N)
# for(ep in 3:200) {
#     model_file = sprintf("%s/%d_model", workdir, ep)
#     if(file.exists(sprintf("%s.bin",model_file))) {
#         W = loadMatrix(model_file,1)
#         pmean = loadMatrix(model_file,3)
#         for(ni in 1:N) {
#             Wacc[[ni]] = cbind(Wacc[[ni]], W[ni,])    
#             pacc[[ni]] = c(pacc[[ni]], c(pmean[ni,]))
#         }
#         
#     }
# }
# 
# gr_pl(t(Wacc[[1]]))