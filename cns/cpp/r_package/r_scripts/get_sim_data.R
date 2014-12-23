#!/usr/bin/RScript

library(Rsnn)
library(rjson)

rundir="/home/alexeyche/prog/newsim/runs"
#rundir="/home/kayla/alexeyche/sim/runs"

runname = system(sprintf("ls -t %s | sed -ne '1p'", rundir),intern=TRUE)
workdir=sprintf("%s/%s", rundir, runname)
for(ep in 1:1000) {
    output_spikes = sprintf("%s/%s_output_spikes.pb", workdir, ep)
    if(!file.exists(output_spikes)) { ep=ep-1; break }
}
#ep = 1
ep_str=""
if(ep>0) {
    ep_str = sprintf("%d_",ep)
}


output_spikes = sprintf("%s/%soutput_spikes.pb", workdir, ep_str)
input_spikes = sprintf("%s/input_spikes.pb", workdir)
stat_file = sprintf("%s/%sstat.pb", workdir, ep_str)
p_stat_file = sprintf("%s/%sp_stat.pb", workdir, ep_str)
model_file = sprintf("%s/%smodel.pb", workdir, ep_str)
const = sprintf("%s/const.json", workdir)

system(sprintf("sed -i -e 's|//.*$||g' %s", const), intern=TRUE)
c = fromJSON(file=const, unexpected.escape ="skip")


if(file.exists(output_spikes)) {
    lab_spikes = RProto$new(output_spikes)$read()
} else 
if (file.exists(input_spikes)) {
    lab_spikes = RProto$new(input_spikes)$read()
}
net = lab_spikes$spikes
    
measure_corr = FALSE
if(measure_corr) {
    N = sum(sapply(c$sim_configuration$input_layers_conf, function(x) x$size))
    dt = c$sim_conf$sim_run_conf$dt
    
    cor_m = measureSpikeCor(net[1:N], dt)
    gr_pl(cor_m)
}




Ti=0
Trange=1000
p1 = prast(net,T0=Ti*Trange,Tmax=(Ti+1)*Trange)

if(file.exists(model_file)) {
    model = RProto$new(model_file)$read()
    #w = w[1:200,101:300]
    w = model[["w"]]
    p2 = levelplot(t(w), col.regions=colorRampPalette(c("black", "white")))
    print(p1, position=c(0, 0.7, 1, 1), more=TRUE)
    print(p2, position=c(0, 0, 1, 0.7))    
} else {
    print(p1)
}






if( (file.exists(stat_file))&&(file.info(stat_file)$size>0)) {
    listen_neuron = c$sim_configuration$neurons_to_listen
    
    stat = RProto$new(stat_file)$read()
    neuron_to_read = 2
    
    nstat_id = grep("NeuronStat", names(stat))
    if(length(nstat_id)>0) {
        nid = nstat_id[neuron_to_read]
        t_plot = 1:100
        
        nst = stat[[nid]]
#         par(mfrow=c(4,1))
#         plot(nst[["u"]][t_plot], type="l")
#         plot(nst[["p"]][t_plot], type="l")
#         plot(nst[["M"]][t_plot], type="l")
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
        syn_id = 15
        
        neuron_to_plot = listen_neuron[1]
        #par(mfrow=c(4,1))
        #sp = net[[neuron_to_plot+1]][ net[[neuron_to_plot+1]]<max(t_plot) ]
        #sp = sp[ sp > min(t_plot) ]
        #plot(sp, rep(1, length(sp)), xlim=c(min(t_plot), max(t_plot)))
#         plot(C[[syn_id]][t_plot], type="l")
#         plot(B[t_plot], type="l")
#         plot(nst[["w"]][[syn_id]][t_plot], type="l")
#         syns = nst[["syns"]]
#         plot(syns[[syn_id]][t_plot],type="l", ylim=c(0,3))
         
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
        syn_id = 58
        
        #par(mfrow=c(3,1))
        
        #plot(nst[["w"]][[syn_id]][t_plot], type="l")            
        #plot(stdp[["y_trace"]][t_plot], type="l")
        #plot(stdp[["x_trace"]][[syn_id]][t_plot], type="l")
    }

}

if( (file.exists(p_stat_file))&&(file.info(p_stat_file)$size>0)) {
    p_stat = RProto$new(p_stat_file)$read()
    p_m = do.call(rbind, lapply(p_stat, function(x) x$p))
    t=1
    pairs = list()
    for(t_end in lab_spikes$end_of_patterns) {
        pairs[[length(pairs)+1]] = c(t, min(t_end, ncol(p_m)) )
        t = t_end + 1
    }
    dist = matrix(0, ncol=length(pairs), nrow=length(pairs))
    i = 1
    for(pi in pairs) {
        j = 1
        for(pj in pairs) {
            ifrom = pi[1]
            ito = pi[2]
            
            jfrom = pj[1]
            jto = pj[2]
            m1 = p_m[,jfrom:jto]
            m2 = p_m[,ifrom:ito]
            if(ncol(m1) > ncol(m2)) {
                dc = ncol(m1) - ncol(m2)
                m2 = cbind(m2, matrix(0, ncol=dc, nrow=nrow(m2)))
            } else
            if(ncol(m1) < ncol(m2)) {
                dc = ncol(m2) - ncol(m1)
                m1 = cbind(m1, matrix(0, ncol=dc, nrow=nrow(m1)))
            }
            d = (m1 - m2)^2
            dist[i,j] = mean(rowSums(d))
            j = j + 1
        }
        i = i + 1
    }
    
    fit = cmdscale(dist, 2, eig=TRUE)
    x <- fit$points[,1]
    y <- fit$points[,2]
    plot(x, y, xlab="Coordinate 1", ylab="Coordinate 2", 
         main="Metric    MDS",	type="n")
    lab_cols = rainbow(length(lab_spikes$labels))
    text(x, y, labels = lab_spikes$labels[lab_spikes$labels_id_timeline+1], cex=.7, col=lab_cols[lab_spikes$labels_id_timeline+1])
    
}
#dev.off()