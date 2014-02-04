#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = FALSE)
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
if(grep("--no-verbose", args)>0) {
    verbose=FALSE
}

system(sprintf("find %s -maxdepth 1 -name \"*.png\" -type f -exec rm -f {} \\;", dir))

#===================================================================================================

Sys.setenv("DISPLAY"=":0.0")

source('util.R')
source('plot_funcs.R')
source('ucr_ts.R')
source('gen_spikes.R')
source('neuron.R')
source('learn_and_run_net.R')
source('srm.R')
source('grad_funcs.R')
source('serialize_to_bin.R')
source('eval_funcs.R')
source('layers.R')
source('kernel.R')

constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest)

ID_MAX=0

set.seed(1234)
data = synth # synthetic control
c(train_dataset, test_dataset) := read_ts_file(data, data_dir)
elems = samples_from_dataset
train_dataset = train_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150,elems),
                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150, elems),
                              sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]


perf = ucr_test(train_dataset, test_dataset, eucl_dist_alg, verbose=FALSE)
if(verbose)
    cat("baseline:", perf$rate, "\n")

start_w.M = matrix(rnorm( M*N, mean=start_w.M.mean, sd=start_w.M.sd), ncol=N, nrow=M)
start_w.N = matrix(rnorm( (N-1)*N, mean=start_w.N.mean, sd=start_w.N.sd), ncol=N, nrow=(N-1))

gr1 = TSNeurons(M = M)
gr2 = TSNeurons(M = M, ids_c = 1000:(1000+M))


neurons = SRMLayer(N, start_w.N, p_edge_prob=net_edge_prob, ninh=ceiling(N*inhib_frac))

gr1$loadPatterns(train_dataset, duration, dt, lambda=5)
gr2$loadPatterns(test_dataset, duration, dt, lambda=5)
patt_len = length(gr1$patterns)

connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
for(i in 1:net_neurons_for_input) {
  cc = sample(gr1$ids, M-afferent_per_neuron)
  connection[cc,i] = 0
}
connection[,(net_neurons_for_input+1):N] = 0

neurons$connectFF(connection, start_w.M, 1:N )

runmode="learn"
test_trials=1

run_options = list(T0 = 0, Tmax = duration, dt = dt, 
                   learning_rate = lr, epochs = epochs, start_epoch = 1, weight_decay = 0, weights_norm_type = weights_norm_type,
                   reward_learning=FALSE,
                   fp_window = 30, fp_kernel_size = 15, 
                   learn_window_size = learn_window_size, mode=runmode, collect_stat=TRUE, 
                   target_set = list(depress_null=FALSE),
                   learn_layer_id = 1,
                   test_patterns = gr2$patterns, 
                   test_function = function(train_set, test_set) {
                      
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
      neurons$weights[[id]] = W[id_to_conn, id] 
      neurons$id_conns[[id]] = id_to_conn
    }))
    cat("Load - Ok\n")
  } else {
    cat("Can't find file for model ", model_file, "\n")
  }
}

layers = SimLayers( list(neurons) )
input_neurons = gr1

loss = run_net(gr1, layers, run_options, verbose=verbose)

W = get_weights_matrix(list(neurons))
if(runmode == "learn") {
  saveMatrixList(model_file, list(W))
}
cat(min(loss), "\n")
