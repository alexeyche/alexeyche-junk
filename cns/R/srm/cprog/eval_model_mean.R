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
ep = 30
if(length(epoch_opt) > 0) {
    ep = epoch_opt
}

srm_sim_exec = sprintf("%s/cns/c/bin/srm_sim", gitdir)
jobs = 5

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

evaldir = sprintf("%s/eval_mean", workdir)
dir.create(file.path(evaldir), showWarnings = FALSE)

evalepdir = sprintf("%s/%s", evaldir, ep)
dir.create(file.path(evalepdir), showWarnings = FALSE)

model_file = sprintf("%s/%s_model", workdir, ep)

################

runs = 10
kernel_param = seq(10, 200, by=10)



for(ri in 1:runs) {
    
    output_file = sprintf("%s/%s_output_spikes", evalepdir, ri)
    test_output_file = sprintf("%s/%s_test_output_spikes", evalepdir, ri)
    
    system(    sprintf("%s -c %s -i %s.bin -o %s.bin -l no -ml %s.bin -j %s", 
                       srm_sim_exec, 
                       const_ini, 
                       input_file, 
                       output_file,
                       model_file,
                       jobs)
    )
    system(    sprintf("%s -c %s -i %s.bin -o %s.bin -l no -ml %s.bin -j %s", 
                       srm_sim_exec, 
                       const_ini, 
                       test_input_file, 
                       test_output_file,
                       model_file,
                       jobs)
    )
}

rates = NULL
for(param_i in 1:length(kernel_param)) {
    train_resps = list()
    test_resps = list()
    for(ri in 1:runs) {
        Tbr = kernel_param[param_i]
        output_file = sprintf("%s/%s_output_spikes", evalepdir, ri)
        test_output_file = sprintf("%s/%s_test_output_spikes", evalepdir, ri)
        
        train_net = getSpikesFromMatrix(loadMatrix(output_file,1))
        test_net = getSpikesFromMatrix(loadMatrix(test_output_file,1))
        
        train_resp = decomposePatterns(train_net[Nids], timeline, labels)
        test_resp = decomposePatterns(test_net[Nids], test_timeline, test_labels)  
        
        train_resp_k = lapply(train_resp, function(d) histKernel(d, list(Tmax=duration, Tbr=Tbr)))
        test_resp_k = lapply(test_resp, function(d) histKernel(d, list(Tmax=duration, Tbr=Tbr)))        
        train_resps[[ri]] = train_resp_k
        test_resps[[ri]] = test_resp_k
    }
    train_means = list()
    test_means = list()
    for(el_i in 1:length(train_resps[[1]])) {
        train_means[[el_i]] = train_resps[[1]][[el_i]]
        for(ri in 2:runs) {
            train_means[[el_i]]$data = train_means[[el_i]]$data + train_resps[[ri]][[el_i]]$data
        }
        train_means[[el_i]]$data = train_means[[el_i]]$data/runs   
    }
    for(el_i in 1:length(test_resps[[1]])) {
        test_means[[el_i]] = test_resps[[1]][[el_i]]
        for(ri in 2:runs) {
            test_means[[el_i]]$data = test_means[[el_i]]$data + test_resps[[ri]][[el_i]]$data
        }
        test_means[[el_i]]$data = test_means[[el_i]]$data/runs   
    }
    c(r, confm_data) := ucr_test(train_means, test_means, eucl_dist_alg, FALSE)
    rates = c(rates, r)
}


print(min(rates))
saveMatrixList(sprintf("%s/eval_output", evalepdir) ,
               list(matrix(rates)) )
#plot(kernel_param, rates, type="l")
#print(confm)
