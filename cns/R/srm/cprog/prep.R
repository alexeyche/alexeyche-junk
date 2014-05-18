#!/usr/bin/RScript
setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")

library(snn)

rundir="/home/alexeyche/prog/sim/runs"
runname = "prep"
workdir=sprintf("%s/%s", rundir, runname)
#runnums = 52
runnums = 1:278
for(runnum in runnums) {
    stat_file = sprintf("%s/%s_stat", workdir, runnum)
    output_spikes = sprintf("%s/%s_output_spikes", workdir, runnum)
    const_ini = sprintf("%s/constants.ini", workdir)
    data = loadMatrix(sprintf("~/prog/sim/bci_data_proc/train/%s_bci",runnum),1)
    
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
    png(sprintf("%s/%s.png", workdir, runnum))
    p = plot_rastl(net,T0=0, Tmax=3000)
    print(p)
    dev.off()
}
#V = loadMatrix(stat_file,1)
#w = loadMatrix(stat_file,2)
#prob=27

#plotl(w[prob,1:1000])

