#setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
setwd("~/prog/alexeyche-junk/cns/R/srm/new")
#dir = '~/my/sim'
dir = '~/prog/sim'
system(sprintf("find %s/R -name \"*.png\" -type f -exec rm -f {} \\;", dir))


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
ID_MAX=0
require(snowfall)
if(!sfIsRunning()) {
  sfInit(parallel=TRUE, cpus=10)
  res = sfClusterEval(require('snnSRM'))
}
sfExport('constants')

data = synth # synthetic control
if(!exists('train_dataset')) {
  c(train_dataset, test_dataset) := read_ts_file(data)
}
#train_dataset = train_dataset[c(1,101, 2, 102, 3, 103, 4, 104, 5, 105)] # cut

duration = 100

N = 10
start_w = 2.0
M = 50
dt = 0.5

start_w.M = matrix(rnorm( M*N, mean=2, sd=0.5), ncol=N, nrow=M)
start_w.N = matrix(rnorm( (N-1)*N, mean=2, sd=0.5), ncol=N, nrow=(N-1))


gr1 = TSNeurons(M = M)
neurons = SRMLayer(N, start_w.N)

gr1$loadPatterns(train_dataset, duration, dt, lambda=8)
patt_len = length(gr1$patterns)
#gr1$patterns = gr1$patterns[sample(patt_len)]
#plot_rastl(gr1$patterns[[3]]$data)
neurons$connectFF(gr1$ids, start_w.M)

runmode="learn"
#runmode="run"
run_options = list(T0 = 0, Tmax = duration, dt = dt, learning_rate = 0.001, epochs = 100,
                   learn_window_size = 100, mode=runmode, collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_5spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1
)
ro = run_options # for debug
id_patt = 1

#model_file = sprintf("%s/R/%s_%dx%d_lr%3.1f_lws_%3.1f", dir, data, M, N, run_options$learning_rate, run_options$learn_window_size)

model_file = sprintf("%s/R/%s_%dx%d", dir, data, M, N)

if(file.exists(paste(model_file, ".idx", sep=""))) {  
    W = loadMatrix(model_file, 1)
    invisible(sapply(1:(N), function(id) { 
      neurons$weights[[id]] = W[1:length(neurons$id_conns[[id]]),id] 
    } 
    ))  
} else {
  cat("Can't find file for model ", model_file, "\n")
}

patterns = gr1$patterns
layers = list(gr1, neurons)

run_net(layers, run_options)

W = get_weights_matrix(list(neurons))
if(runmode == "learn") {
  saveMatrixList(model_file, list(W))
}