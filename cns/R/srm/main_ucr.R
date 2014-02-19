#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = FALSE)
if(length(grep("RStudio", args))>0) {
  verbose = TRUE
  dir='~/prog/sim/runs/test'
  #dir='~/my/sim/runs/testrstudio'
  data_dir = '~/prog/sim'
  #data_dir = '~/my/sim'
  setwd("~/prog/alexeyche-junk/cns/R/srm")
  #setwd("~/my/git/alexeyche-junk/cns/R/srm")
  source('constants.R')
} else {
  if(length(args) == 5) {
    cat("Available options: \n")
    cat("\t\t--const-file=%constants.R% constants.R by default\n")
    cat("\t\t--runs-dir=%dir_with_runs% created dir for runs\n")
    cat("\t\t--run-name=%name% name for current run (data by default)\n")
    q()
  }
  base_dir = dirname(substring( args[grep("--file=", args)], 8))
  setwd(base_dir)
  
  const_file = substring(args[grep("--const-file=", args)], 14)
  if(length(const_file) == 0) {
    const_file = paste(c(base_dir, 'constants.R'), collapse='/')
  }
  source(const_file)
  
  runs_dir = substring(args[grep("--runs-dir=", args)], 12)
  if((length(runs_dir) == 0)||(!file.exists(runs_dir))) {
    cat("Need created dir for runs in options (--runs-dir=%directory%)\n")
    q()
  }
  
  run_name = substring(args[grep("--run-name=", args)], 12)
  if(length(run_name) == 0) {
    run_name = format(Sys.time(), "%d_%m_%y_%H_%M")
  }
  
  dir = paste(c(runs_dir, run_name), collapse="/")
  dir.create(file.path(dir), showWarnings = FALSE)
  copy_ok = file.copy(const_file, dir)
  
  temp_spl = strsplit(runs_dir, "/")[[1]]
  data_dir = paste( temp_spl[1:(length(temp_spl)-1)], collapse='/')
  
  if(!file.exists(data_dir)) {
    cat(sprintf("Need ts directory with time series data: %s\n", data_dir))
    q()
  }
  
  verbose=TRUE
  if(length(grep("--no-verbose", args))>0) {
    verbose=FALSE
  }
  Sys.setenv("DISPLAY"=":0.0")  
}
system(sprintf("find %s -maxdepth 1 -name \"*.png\" -type f -exec rm -f {} \\;", dir))
#===================================================================================================

library(snn)

source('srm_funcs.R')
source('plot_funcs.R')
source('ucr_ts.R')
source('learn_and_run_net.R')
source('srm.R')
source('grad_funcs.R')
source('serialize_to_bin.R')
source('eval_funcs.R')
source('kernel.R')


set.seed(seed_num)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, added_lrate = added_lrate, sim_dim=sim_dim)

ID_MAX=0

data = synth # synthetic control
c(train_dataset, test_dataset) := read_ts_file(data, data_dir)
elems = samples_from_dataset
train_dataset = train_dataset[c(sample(1:50, elems), sample(101:150, elems))] #, sample(101:150,elems),
#                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(101:150, elems))]  #, sample(101:150, elems),
                              #sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]

train_dataset = train_dataset[sample(1:length(train_dataset))]

perf = ucr_test(train_dataset, test_dataset, eucl_dist_alg, verbose=FALSE)
if(verbose)
    cat("baseline:", perf$rate, "\n")

start_w.M = matrix(rnorm( M*N, mean=start_w.M.mean, sd=start_w.M.sd), ncol=N, nrow=M)
start_w.N = matrix(rnorm( (N-1)*N, mean=start_w.N.mean, sd=start_w.N.sd), ncol=N, nrow=(N-1))

gr1 = TSNeurons(M = M)
gr2 = TSNeurons(M = M, ids_c = 1000:(1000+M))


n = SRMLayerClass$new(N, start_w.N, p_edge_prob=0.1, ninh=ceiling(N*inhib_frac))

gr1$loadPatterns(train_dataset, duration, dt, lambda=5)
gr2$loadPatterns(test_dataset, duration, dt, lambda=5)
patt_len = length(gr1$patterns)

connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
for(i in 1:net_neurons_for_input) {
  cc = sample(gr1$ids, M-afferent_per_neuron)
  connection[cc,i] = 0
}
if(net_neurons_for_input<N)
  connection[,(net_neurons_for_input+1):N] = 0

n$connectFF(connection, start_w.M, 1:N )

runmode="learn"
test_trials=2

run_options = list(T0 = 0, Tmax = duration, dt = dt, 
                   learning_rate = lr, epochs = epochs, start_epoch = 1, weight_decay = 0, weights_norm_type = weights_norm_type,
                   reward_learning=TRUE,
                   fp_window_size = 10, fp_kernel_size = 5, 
                   learn_window_size = learn_window_size, mode=runmode, collect_stat=TRUE, 
                   target_set = list(depress_null=FALSE),
                   learn_layer_id = 1,
                   seed_num = seed_num,
                   test_patterns = gr2$patterns, 
                   test_function = function(train_set, test_set) {
#                     Ktrain = lapply(train_set, function(act) kernelWindow_spikes(act, list(sigma = ro$fp_kernel_size, window = ro$fp_window_size, T0 = ro$T0, Tmax = ro$Tmax, quad = 256)) )
#                     Ktest = lapply(test_set, function(act) kernelWindow_spikes(act, list(sigma = ro$fp_kernel_size, window = ro$fp_window_size, T0 = ro$T0, Tmax = ro$Tmax, quad = 256)) )
#                     
#                     perf = ucr_test(Ktrain, Ktest, eucl_dist_alg, FALSE)
#                     return(perf$rate)                     
                   }, evalTrial=test_trials, test_run_freq=5
)

# for debug:
ro = run_options 
id_patt = 1
patterns = gr1$patterns

model_file = sprintf("%s/%s_%dx%d", dir, data, M, N)
if(runmode=="run") {
  if(file.exists(paste(model_file, ".idx", sep=""))) {  
    W = loadMatrix(model_file, 1)
    invisible(sapply(1:N, function(id) { 
      id_to_conn = which(W[,id] != 0)
      n$obj$weights[[id]] = W[id_to_conn, id] 
      n$obj$id_conns[[id]] = id_to_conn
    }))
    cat("Load - Ok\n")
  } else {
    cat("Can't find file for model ", model_file, "\n")
  }
}

input_neurons = gr1
ep=id_patt=1

loss = run_net(gr1, n, run_options, verbose=verbose)

W = n$Wm()
if(runmode == "learn") {
  saveMatrixList(model_file, list(W))
}
cat(min(loss), "\n")
