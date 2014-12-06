#!/usr/bin/RScript

library(Rsnn)

#rundir="/home/alexeyche/prog/newsim/runs"
rundir="/home/kayla/alexeyche/sim/runs"
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
model_file = sprintf("%s/%smodel.pb", workdir, ep_str)


net = RProto$new(output_spikes)$read()

Ti=0
Trange=1000
p1 = prast(net,T0=Ti*Trange,Tmax=(Ti+1)*Trange)
#print(p1, position=c(0, 0.6, 1, 1)), more=TRUE)
print(p1)

if(file.exists(model_file)) {
    model = RProto$new(model_file)$read()
    w = model[["w"]]
} 
if(file.exists(stat_file)) {
    stat = RProto$new(stat_file)$read()
    nstat_id = grep("NeuronStat", names(stat))
    if(length(nstat_id)>0) {
        nid = nstat_id[1]
        t_plot = 1:1000
        
        nst = stat[[nid]]
        par(mfrow=c(2,1))
        #plot(nst[["u"]][t_plot], type="l")
        plot(nst[["p"]][t_plot], type="l")
        sp = net[[102]][net[[101]]<1000]
        plot(sp, rep(1, length(sp)), xlim=c(min(t_plot), max(t_plot)))
#         syns = nst[["syns"]]
#         if(length(syns)>0) {
#             plot(syns[[1]][t_plot],type="l", ylim=c(0,3))
#             for(i in 2:length(syns)) {
#                 lines(syns[[i]][t_plot])
#             }
#         }
    }
    rew_stat_id = grep("RewardStat", names(stat))
    if(length(rew_stat_id)>0) {
        lid = rew_stat_id[1]
        rst = stat[[lid]]
        r = rst[["r"]]
        mean_r = rst[["mean_r"]]
        
    }
}


