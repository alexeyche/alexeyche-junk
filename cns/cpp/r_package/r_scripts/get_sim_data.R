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
model = RProto$new(model_file)$read()
w = model[["w"]]

p2 = levelplot(t(w), col.regions=colorRampPalette(c("black", "white")))

print(p1, position=c(0, 0.7, 1, 1), more=TRUE)
print(p2, position=c(0, 0, 1, 0.7))


if(file.exists(stat_file)) {
    stat = RProto$new(stat_file)$read()
    neuron_to_read = 1
    nstat_id = grep("NeuronStat", names(stat))
    if(length(nstat_id)>0) {
        nid = nstat_id[neuron_to_read]
        t_plot = 1:1000
        
        nst = stat[[nid]]
        par(mfrow=c(2,1))
        #plot(nst[["u"]][t_plot], type="l")
        plot(nst[["p"]][t_plot], type="l")
        sp = net[[102]][net[[102]]<1000]
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
    maxl_stat_id = grep("MaxLikelihoodStat", names(stat))
    if(length(maxl_stat_id)>0) {
        id = maxl_stat_id[neuron_to_read]
        maxl = stat[[id]]
        tr = maxl[["traces"]]
        t_plot = 0+1:1000
        syn_id = 70
        par(mfrow=c(3,1))
        #plot(nst[["p"]][t_plot], type="l")
        sp = net[[101]][ net[[101]]<max(t_plot) ]
        sp = sp[ sp > min(t_plot) ]
        plot(sp, rep(1, length(sp)), xlim=c(min(t_plot), max(t_plot)))
        plot(nst[["syns"]][[syn_id]][t_plot], type="l")
        plot(tr[[syn_id]][t_plot], type="l")
    }
}


