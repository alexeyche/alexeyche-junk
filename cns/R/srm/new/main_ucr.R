#setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
setwd("~/prog/alexeyche-junk/cns/R/srm/new")
#dir = '~/prog/sim/0.15run_sec'
#dir = '~/prog/sim'
dir = '~/prog/sim/last3'
#dir = '~/prog/sim/0.125run_sec'
#dir = '/home/alexeyche/my/sim/run_0.175'
#dir = '/home/alexeyche/prog/sim/0.6run_sec'
#dir = '/home/alexeyche/prog/sim/0.6run_sec_low_lr'
#system(sprintf("find %s/R -maxdepth 1 -name \"*.png\" -type f -exec rm -f {} \\;", dir))


source('util.R')
source('plot_funcs.R')
source('ucr_ts.R')
source('gen_spikes.R')
source('neuron.R')

source('target_functions.R')
source('learn_and_run_net.R')
source('srm.R')
source('grad_funcs.R')
source('serialize_to_bin.R')
source('eval_funcs.R')
source('layers.R')
source('kernel.R')

ID_MAX=0
#require(snowfall)
#if(!sfIsRunning()) {
#  sfInit(parallel=TRUE, cpus=10)
#  res = sfClusterEval(require('snnSRM'))
#}
#sfExport('constants')

data = synth # synthetic control
if(!exists('train_dataset')) {
  set.seed(1234)
  c(train_dataset, test_dataset) := read_ts_file(data)
 #train_dataset = train_dataset[c(sample(1:50, 10), sample(51:100, 10), sample(101:150,10),
#                                 sample(151:200, 10), sample(201:250,10), sample(251:300,10))] # cut
# test_dataset = test_dataset[c(sample(1:50, 10), sample(51:100, 10), sample(101:150, 10),
#                               sample(151:200, 10), sample(201:250,10), sample(251:300, 10))]
  
  ucr_test(train_dataset, test_dataset, eucl_dist_alg)
 
}

duration = 300

N = 10
M = 50
dt = 0.5

start_w.M = 10 #matrix(rnorm( M*N, mean=2, sd=0.5), ncol=N, nrow=M)
start_w.N = 5 #matrix(rnorm( (N-1)*N, mean=2, sd=0.5), ncol=N, nrow=(N-1))


gr1 = TSNeurons(M = M)
gr2 = TSNeurons(M = M, ids_c = 1000:(1000+M))
neurons = SRMLayer(N, start_w.N, p_edge_prob=0.5)

gr1$loadPatterns(train_dataset, duration, dt, lambda=5)
gr2$loadPatterns(test_dataset, duration, dt, lambda=5)
patt_len = length(gr1$patterns)
gr1$patterns = gr1$patterns[sample(patt_len)]
#plot_rastl(gr1$patterns[[3]]$data)

connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
connect_window = N*2
step = M/N
overlap = 1
for(ni in 0:(N-1)) {
  if(ni != 0) connection[1:((ni*step)-overlap),ni+1] = 0    
  if(ni != N-1) connection[((ni*step)+step+1+overlap):M,ni+1] = 0
}

neurons$connectFF(connection, start_w.M, 1:N )

runmode="learn"
#runmode="run"
test_trials=5

run_options = list(T0 = 0, Tmax = duration, dt = dt, 
                   learning_rate = 0.01, epochs = 50, start_epoch = 1, weight_decay = 0,
                   reward_learning=FALSE,
                   fp_window = 30, fp_kernel_size = 15, dev_frac_norm = 0.25,
                   learn_window_size = 150, mode=runmode, collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_4spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1,
                   test_patterns = gr2$patterns, 
                   test_function = function(train_set, test_set) {
#                     kernSize=10
#                     train_processed = post_process_set(train_set, test_trials, 0, duration, binKernel, kernSize)
#                     test_processed = post_process_set(test_set, test_trials, 0, duration, binKernel, kernSize)
#                     perf = ucr_test(train_processed, test_processed, eucl_dist_alg)
#                     return(perf$rate)
                   }, trials=test_trials, test_run_freq=5
)
ro = run_options # for debug
id_patt = 1

#model_file = sprintf("%s/R/%s_%dx%d_lr%3.1f_lws_%3.1f", dir, data, M, N, run_options$learning_rate, run_options$learn_window_size)

model_file = sprintf("%s/R/%s_%dx%d", dir, data, M, N)
if(runmode=="run") {
  model_file = "/home/alexeyche/prog/sim/last2/R/synthetic_control_50x10_8"
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
patterns = gr1$patterns
layers = SimLayers( list(neurons) )
input_neurons = gr1

run_net(gr1, layers, run_options)

W = get_weights_matrix(list(neurons))
if(runmode == "learn") {
  saveMatrixList(model_file, list(W))
}
