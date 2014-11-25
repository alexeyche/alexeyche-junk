#!/usr/bin/RScript

library(Rsnn)

rundir="/home/alexeyche/prog/newsim/runs"
runname = system(sprintf("ls -t %s | sed -ne '1p'", rundir),intern=TRUE)
workdir=sprintf("%s/%s", rundir, runname)
for(ep in 1:1000) {
    output_spikes = sprintf("%s/%s_output_spikes.pb", workdir, ep)
    if(!file.exists(output_spikes)) { ep=ep-1; break }
}
ep_str=""
if(ep>0) {
    ep_str = sprintf("%d_",ep)
}

output_spikes = sprintf("%s/%soutput_spikes.pb", workdir, ep_str)
stat_file = sprintf("%s/%sstat.pb", workdir, ep_str)


net = RProto$new(output_spikes)$read()

Ti=0
Trange=1000
p1 = prast(net,T0=Ti*Trange,Tmax=(Ti+1)*Trange)
#print(p1, position=c(0, 0.6, 1, 1)), more=TRUE)
print(p1)

# 
# 
if(file.exists(stat_file)) {
    stat = RProto$new(stat_file)$read()
    nid=1
    t_plot = 1:100
    
    nst = stat[[nid]]
    par(mfrow=c(3,1))
    plot(nst[["u"]][t_plot], type="l")
    plot(nst[["p"]][t_plot], type="l")
    syns = nst[["syns"]]
    if(length(syns)>0) {
        plot(syns[[1]][t_plot],type="l", ylim=c(0,3))
        for(i in 2:length(syns)) {
            lines(syns[[i]][t_plot])
        }
    }
}

