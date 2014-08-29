#!/usr/bin/RScript
setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")
#setwd("~/my/git/alexeyche-junk/cns/R/srm/cprog")
source('../serialize_to_bin.R')
source('../util.R')

source('../plot_funcs.R')


library(snn)

rundir="/home/alexeyche/prog/sim/runs"
#rundir="/home/alexeyche/prog/sim/spear_runs"
#runname = "PoissonLayer_TripleSTDP_ExpHennequin_0003"

runname = system(sprintf("ls -t %s | head -n1", rundir),intern=TRUE)

workdir=sprintf("%s/%s", rundir, runname)


for(ep in 1:1000) {
    output_spikes = sprintf("%s/%s_output_spikes.bin", workdir, ep)
    if(!file.exists(output_spikes)) { ep=ep-1; break }
}
#
#ep=500

ep_str=""
if(ep>0) {
    ep_str = sprintf("%d_",ep)
}
model_file = sprintf("%s/%smodel", workdir, ep_str)
stat_file = sprintf("%s/%sstat", workdir, ep_str)
output_spikes = sprintf("%s/%soutput_spikes", workdir, ep_str)
const_ini = sprintf("%s/constants.ini", workdir)


N = as.integer(get_const("N"))
M = as.integer(get_const("M"))
lrule = get_const("learning_rule")

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

Ti=0
Trange=1000
p1 = plot_rastl(net[(M-M+1):(M+sum(N))],T0=Ti*Trange,Tmax=(Ti+1)*Trange)

if(file.exists(sprintf("%s.bin",stat_file))) {
    if(get_const("reinforcement") == "true") {
        rew_all = c()
        for(ep in 1:1000) {
            stat_file = sprintf("%s/%d_stat", workdir, ep)
            if(file.exists(sprintf("%s.bin",stat_file))) {
                rew = loadMatrix(stat_file, 1)                
                rew_all = c(rew_all, mean(rew[1,]))
            } else {
                break
            }
        }
        if(!is.nan(rew_all)) {
            plot(rew_all,type="l")
        }
    } else {
        it=1
        p = loadMatrix(stat_file, it); it=it+1
        u = loadMatrix(stat_file, it); it=it+1
        
        
        syn=11
        nid=10
        Tplot=1:1000
        
        #syns = loadMatrix(stat_file, it+nid); it=it+N
        #dWn = loadMatrix(stat_file, it+nid); it=it+N
        
        if(lrule == "OptimalSTDP") {            
            B = loadMatrix(stat_file, 2+2*N+1)
            Cn = loadMatrix(stat_file, 2+2*N+1+nid)
            par(mfrow=c(3,1))
            spikes = net[[M+nid]][net[[M+nid]]<max(Tplot)]
            plot(spikes, rep(1,length(spikes)), xlim=c(min(Tplot),max(Tplot)) )
            plotl(syns[syn,Tplot])
            plotl(Cn[syn,Tplot])
            #plotl(dWn[syn,Tplot])
        }
        if(lrule == "ResourceSTDP") {
            res = loadMatrix(stat_file, 3)
            y_tr = loadMatrix(stat_file, 4)
            x_tr = loadMatrix(stat_file, 4+nid)
            syns = loadMatrix(stat_file, 4+N+nid)
            dWn = loadMatrix(stat_file, 4+2*N+nid)
            par(mfrow=c(4,1))
            spikes = net[[M+nid]][net[[M+nid]]<max(Tplot)]
            plotl(y_tr[nid,Tplot])
            plotl(x_tr[syn,Tplot])
            plotl(res[nid,Tplot])
            plotl(dWn[syn,Tplot])
        }
        if(lrule == "TripleSTDP") {    
          o_one = loadMatrix(stat_file, it); it=it+1
          o_two = loadMatrix(stat_file, it); it=it+1
          a_minus = loadMatrix(stat_file, it); it=it+1
          pacc = loadMatrix(stat_file, it); it=it+1
          r = loadMatrix(stat_file, it+nid)
          par(mfrow=c(4,1))
          spikes = net[[M+nid]][net[[M+nid]]<max(Tplot)]
          plot(spikes, rep(1,length(spikes)), xlim=c(min(Tplot),max(Tplot)) )
          #plotl(o_one[nid,Tplot])
          plotl(o_two[nid,Tplot])
          plotl(pacc[nid,Tplot])
          plotl(a_minus[nid,Tplot])
          #plotl(dWn[syn,Tplot])
          #plotl(syns[syn,Tplot])
        }
    }
} else {
    if(lrule == "OptimalSTDP") { 
      matrix_per_layer = 8
    } else 
    if(lrule == "SimpleSTDP") { 
        matrix_per_layer = 6
    }
    Wnorm = W = NULL
    max_row = sum(sum(N)+M)
    for(Ni in 1:length(N)) {
        Wlayer = loadMatrix(model_file,(Ni-1)*matrix_per_layer+1)
        if(ncol(Wlayer)<max_row) {
            Wlayer = cbind(Wlayer, matrix(0, nrow=nrow(Wlayer), ncol=(max_row-ncol(Wlayer))))
        }
        Wnorm = rbind(Wnorm, Wlayer/max(Wlayer))
        
        W = rbind(W, Wlayer)
    }
    
    Wnorm = W
    p2 = levelplot(t(Wnorm), col.regions=colorRampPalette(c("black", "white")))
    
    print(p1, position=c(0, 0.6, 1, 1), more=TRUE)
    print(p2, position=c(0, 0, 1, 0.6))
    
    #hist(W[101:200,])
    
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
}    