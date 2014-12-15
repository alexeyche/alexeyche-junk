#!/usr/bin/RScript

library(Rsnn)

rundir="/home/alexeyche/prog/newsim/runs"
#rundir="/home/kayla/alexeyche/sim/runs"

runname = system(sprintf("ls -t %s | sed -ne '1p'", rundir),intern=TRUE)
workdir=sprintf("%s/%s", rundir, runname)
for(ep in 1:1000) {
    output_spikes = sprintf("%s/%s_output_spikes.pb", workdir, ep)
    if(!file.exists(output_spikes)) { ep=ep-1; break }
}
ep = 1
ep_str=""
if(ep>0) {
    ep_str = sprintf("%d_",ep)
}

output_spikes = sprintf("%s/%soutput_spikes.pb", workdir, ep_str)
stat_file = sprintf("%s/%sstat.pb", workdir, ep_str)
model_file = sprintf("%s/%smodel.pb", workdir, ep_str)
const = sprintf("%s/const.json", workdir)

net = RProto$new(output_spikes)$read()

Ti=0
Trange=1000
p1 = prast(net,T0=Ti*Trange,Tmax=(Ti+1)*Trange)
#print(p1, position=c(0, 0.6, 1, 1)), more=TRUE)
model = RProto$new(model_file)$read()
w = model[["w"]]
#w = w[1:200,101:300]

p2 = levelplot(t(w), col.regions=colorRampPalette(c("black", "white")))

print(p1, position=c(0, 0.7, 1, 1), more=TRUE)
print(p2, position=c(0, 0, 1, 0.7))


if( (file.exists(stat_file))&&(file.info(stat_file)$size>0)) {
    library(rjson)
    system(sprintf("sed -i -e 's|//.*$||g' %s", const), intern=TRUE)
    c = fromJSON(file=const, unexpected.escape ="skip")
    
    listen_neuron = c$sim_configuration$neurons_to_listen
    
    stat = RProto$new(stat_file)$read()
    neuron_to_read = 1
    
    nstat_id = grep("NeuronStat", names(stat))
    if(length(nstat_id)>0) {
        nid = nstat_id[neuron_to_read]
        t_plot = 1:1000
        
        nst = stat[[nid]]
#         par(mfrow=c(3,1))
#         plot(nst[["u"]][t_plot], type="l")
#         plot(nst[["p"]][t_plot], type="l")
#         #sp = net[[listen_neuron]][net[[listen_neuron]]<1000]
#         #plot(sp, rep(1, length(sp)), xlim=c(min(t_plot), max(t_plot)))
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
        
        rst = list(r=NULL, mean_r=NULL)
        for(old_ep in 1:1000) {
            old_stat_file = sprintf("%s/%s_stat.pb", workdir, old_ep)
            if(file.exists(old_stat_file)) {
                old_stat = RProto$new(old_stat_file)$read()
                old_rst = old_stat[[lid]]
                rst[["r"]] = c(rst[["r"]], old_rst[["r"]])
                rst[["mean_r"]] = c(rst[["mean_r"]], old_rst[["mean_r"]])
            } else {
                break
            }
        }            
       
        #rst = stat[[lid]]
        r = rst[["r"]]
        mean_r = rst[["mean_r"]]
        #plotl(r-mean_r)
    }
    maxl_stat_id = grep("MaxLikelihoodStat", names(stat))
    if(length(maxl_stat_id)>0) {
        id = maxl_stat_id[neuron_to_read]
        maxl = stat[[id]]
        tr = maxl[["traces"]]
        t_plot = 0+1:1000
        syn_id = 33
        par(mfrow=c(3,1))
        #plot(nst[["p"]][t_plot], type="l")
        inp = any(sapply(c$learning_rules, function(r) if("input_target" %in% names(r)) { r$input_target } else { FALSE } ))
        neuron_to_plot = listen_neuron
        if(inp) {
            neuron_to_plot = listen_neuron - sum(sapply(c$sim_conf$input_layers_conf, function(l) l$size))
            syn_id = neuron_to_plot
        }
        sp = net[[neuron_to_plot+1]][ net[[neuron_to_plot+1]]<max(t_plot) ]
        sp = sp[ sp > min(t_plot) ]
        plot(sp, rep(1, length(sp)), xlim=c(min(t_plot), max(t_plot)))
        plot(nst[["syns"]][[syn_id+1]][t_plot], type="l")
        plot(tr[[syn_id]][t_plot], type="l")
    }
    opt_stdp_stat_id = grep("OptimalStdpStat", names(stat))
    mean_p_dur_min = min(sapply(c$learning_rules, function(x) if("mean_p_dur" %in% names(x)) x$mean_p_dur else Inf))
    if((length(opt_stdp_stat_id)>0)&&(model[["sim_time"]]>=mean_p_dur_min)) {
        id = opt_stdp_stat_id[neuron_to_read]
        opt = stat[[id]]
        C = opt[["C"]]
        B = opt[["B"]]
        if(length(B) == 0) return;
        t_plot = 0+1:1000
        syn_id = 40
        par(mfrow=c(4,1))
        neuron_to_plot = listen_neuron[1]
        #sp = net[[neuron_to_plot+1]][ net[[neuron_to_plot+1]]<max(t_plot) ]
        #sp = sp[ sp > min(t_plot) ]
        #plot(sp, rep(1, length(sp)), xlim=c(min(t_plot), max(t_plot)))
        plot(C[[syn_id]][t_plot], type="l")
        plot(B[t_plot], type="l")
        plot(nst[["w"]][[syn_id]][t_plot], type="l")
        syns = nst[["syns"]]
        plot(syns[[syn_id]][t_plot],type="l", ylim=c(0,3))
         
#         if(length(syns)>0) {
#             plot(syns[[1]][t_plot],type="l", ylim=c(-3,3))
#             for(i in 2:length(syns)) {
#                 lines(syns[[i]][t_plot])
#             }
#         }
    }
    stdp_stat_id = grep("^StdpStat", names(stat)) 
    if(length(stdp_stat_id)>0) {
        id = stdp_stat_id[neuron_to_read]
        stdp = stat[[id]]
        syn_id = 30
        
        par(mfrow=c(3,1))
        
        plot(nst[["w"]][[syn_id]][t_plot], type="l")            
        plot(stdp[["y_trace"]][t_plot], type="l")
        plot(stdp[["x_trace"]][[syn_id]][t_plot], type="l")
    }

}

