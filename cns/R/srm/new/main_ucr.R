setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
dir = '~/my/sim'



source('util.R')
source('plot_funcs.R')
source('ucr_ts.R')
source('gen_spikes.R')
source('neuron.R')

source('target_functions.R')
source('learn_and_run_net.R')
source('srm.R')

data = synth # synthetic control
if(!exists('train_dataset')) {
  c(train_dataset, test_dataset) := read_ts_file(data)
}


duration = 300

N = 10
start_w = 1.5
M = 20
dt = 0.5
if(!exists('gr1')) {
  gr1 = TSNeurons(M = M)
  gr1$loadPatterns(train_dataset, duration, dt, lambda=4)
}
#plot_rastl(gr1$patterns[[3]]$data)

neurons = SRMLayer(N, start_w/4)
neurons$connectFF(gr1$ids, start_w)

run_options = list(T0 = 0, Tmax = duration, dt = dt, learning_rate = 0.5, epochs = 100,
                   learn_window_size = duration, mode="run", collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_2spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1
)


#model_file = sprintf("%s/%s_%dx%d_lr%3.1f_lws_%3.1f", dir, data, M, N, run_options$learning_rate, run_options$learn_window_size)

model_file = sprintf("%s/%s_%dx%d", dir, data, M, N)

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
layers = list(neurons)

run_net(layers, patterns, run_options)
