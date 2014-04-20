args <- commandArgs(trailingOnly = FALSE)
epoch_opt = substring( args[grep("--epoch=", args)], 9)

setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")
#setwd("~/my/git/alexeyche-junk/cns/R/srm/cprog")
library(snn)

source('../eval_funcs.R')
source('../ucr_ts.R')
source('../plot_funcs.R')

ucr_spikes_dir = "/home/alexeyche/prog/sim/ucr_spikes_full"
#ucr_spikes_dir = "/home/alexeyche/my/sim/ucr_spikes"
gitdir = "/home/alexeyche/prog/alexeyche-junk"
#gitdir = "/home/alexeyche/my/git/alexeyche-junk"
rundir = "/home/alexeyche/prog/sim/runs"
#rundir = "/home/alexeyche/my/sim/runs"
#runname="test_run"
#runname = "n50_no_conn"
#runname = "n50_conn_3"
runname = "n100_full"
ep = 20
if(length(epoch_opt) > 0) {
    ep = epoch_opt
}

srm_sim_exec = sprintf("%s/cns/c/bin/srm_sim", gitdir)
jobs = 8

workdir = sprintf("%s/%s", rundir, runname)

const_ini = sprintf("%s/constants.ini", workdir)
M = as.integer(get_const("M"))
N = as.integer(get_const("N"))
Mids=1:M
Nids=(M+1):(M+N)


input_file = sprintf("%s/train/1_ucr_50elems_6classes_1000dur", ucr_spikes_dir)
timeline = c(loadMatrix(input_file,2))
labels = c(loadMatrix(input_file,3))
test_input_file =  sprintf("%s/test/ucr_50elems_6classes_1000dur", ucr_spikes_dir)
test_timeline = c(loadMatrix(test_input_file,2))
test_labels = c(loadMatrix(test_input_file,3))
duration = timeline[2]-timeline[1]

evaldir = sprintf("%s/eval", workdir)
dir.create(file.path(evaldir), showWarnings = FALSE)

evalepdir = sprintf("%s/%s", evaldir, ep)
dir.create(file.path(evalepdir), showWarnings = FALSE)

model_file = sprintf("%s/%s_model", workdir, ep)

################

#tresholds = seq(-65, -40, by=2.5)
tresholds = seq(5, 20, by=2.5)
#tresholds = c(12)
sigmas = seq(0.1,10, length.out=10)
kernel_param = seq(10, 200, by=10)
#tresholds = c(-60)
#sigmas = c(1)

rates = matrix(0, nrow=length(tresholds), ncol=length(kernel_param))

for(tr_i in 1:length(tresholds)) {
    tr = tresholds[tr_i]
    
    run_const_ini = sprintf("%s/constants.ini.%s", evalepdir, tr_i)
    system( sprintf("cp %s %s", const_ini, run_const_ini))
    patch_const(run_const_ini, "tr", tr)
    patch_const(run_const_ini, "determ", "true")
    patch_const(run_const_ini, "learn", "false")
    
    output_file = sprintf("%s/%s_output_spikes", evalepdir, tr_i)
    test_output_file = sprintf("%s/%s_test_output_spikes", evalepdir, tr_i)
    
    system(    sprintf("%s -c %s -i %s.bin -o %s.bin -l no -ml %s.bin -j %s", 
                       srm_sim_exec, 
                       run_const_ini, 
                       input_file, 
                       output_file,
                       model_file,
                       jobs)
    )
    system(    sprintf("%s -c %s -i %s.bin -o %s.bin -l no -ml %s.bin -j %s", 
                       srm_sim_exec, 
                       run_const_ini, 
                       test_input_file, 
                       test_output_file,
                       model_file,
                       jobs)
    )
}

conf_matrices = list()
for(tr_i in 1:length(tresholds)) {
    conf_matrices[[tr_i]] = list()
    for(param_i in 1:length(kernel_param)) {
        tr = tresholds[tr_i]
        Tbr = kernel_param[param_i]
        output_file = sprintf("%s/%s_output_spikes", evalepdir, tr_i)
        test_output_file = sprintf("%s/%s_test_output_spikes", evalepdir, tr_i)
        
        train_net = getSpikesFromMatrix(loadMatrix(output_file,1))
        test_net = getSpikesFromMatrix(loadMatrix(test_output_file,1))
        
        train_resp = decomposePatterns(train_net[Nids], timeline, labels)
        test_resp = decomposePatterns(test_net[Nids], test_timeline, test_labels)
        
        train_resp_k = lapply(train_resp, function(d) histKernel(d, list(Tmax=duration, Tbr=Tbr)))
        test_resp_k = lapply(test_resp, function(d) histKernel(d, list(Tmax=duration, Tbr=Tbr)))
        
        c(r, confm_data) := ucr_test(train_resp_k, test_resp_k, eucl_dist_alg, FALSE)
        labs = unique(labels)
        confm = matrix(0, length(labs), length(labs))
        for(d in confm_data) {
            confm[d$pred, d$true] = confm[d$pred, d$true] + 1
        }
        conf_matrices[[tr_i]][[param_i]] = confm
        rates[tr_i, param_i] = r
        cat("tr: ", tr, "Tbr: ", Tbr, " rate: ", r, "\n")
    }
}
inds = which(rates == min(rates), arr.ind=TRUE)

saveMatrixList(sprintf("%s/eval_output", evalepdir) ,
               list(rates, matrix(tresholds), matrix(sigmas), conf_matrices[[ inds[,1] ]][[ inds[,2] ]]) )
gr_pl(t(rates))
#print(confm)
