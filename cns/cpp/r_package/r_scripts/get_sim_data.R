#!/usr/bin/RScript

library(Rsnn)
library(rjson)
setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")


rundir="/home/alexeyche/prog/newsim/runs"
#rundir="/home/kayla/alexeyche/sim/runs"
#rundir="/home/alexeyche/prog/sim_spear/eval_clustering_p_stat_optimal_stdp"
#rundir="/home/alexeyche/prog/sim_spear/eval_clustering_p_stat_structure"
runname = system(sprintf("ls -t %s | sed -ne '1p'", rundir),intern=TRUE)
runname = "34"
workdir=sprintf("%s/%s", rundir, runname)
for(ep in 1:1000) {
    output_spikes = sprintf("%s/%s_output_spikes.pb", workdir, ep)
    if(!file.exists(output_spikes)) { ep=ep-1; break }
}

#ep = 1
ep_str="0_"
if(ep>0) {
    ep_str = sprintf("%d_",ep)
}

#output_spikes = sprintf("%s/%stune_weights_output_spikes.pb", workdir, ep_str)
output_spikes = sprintf("%s/%soutput_spikes.pb", workdir, ep_str)
input_spikes = sprintf("%s/input_spikes.pb", workdir)
stat_file = sprintf("%s/%sstat.pb", workdir, ep_str)
p_stat_file = sprintf("%s/%sp_stat.pb", workdir, ep_str)
proc_out_json = sprintf("%s/%sproc_output.json", workdir, ep_str)
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




Ti=100
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
    stat = RProto$new(stat_file)$read()
    source('plot_stat.R')
    plot_stat(stat, c, net, model, 2, 50, 1:1000)

}
p_stat_eval = TRUE
if(p_stat_eval) {
    if( (file.exists(proc_out_json))&&(file.info(proc_out_json)$size>0)) {
        setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")
        source('../../scripts/eval_dist_matrix.R')
        #calculate_criterion(proc_out_json)
    }
}
