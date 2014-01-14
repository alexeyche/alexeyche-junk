#!/usr/bin/RScript
#setwd("~/prog/alexeyche-junk/cns/R/srm/new")
setwd("~/my/git/alexeyche-junk/cns/R/srm/new")

source('include.R')

if(!sfIsRunning()) {
  sfInit(parallel=TRUE, cpus=10)
  res = sfClusterEval(require('snnSRM'))
}

sfExport('constants')
#dir = "/home/alexeyche/prog/sim/R"
dir = "/home/alexeyche/my/sim"
system(sprintf("find %s -name \"*.png\" -type f -exec rm -f {} \\;", dir))

M = 50
N = 10
Nro = 2
gr1 = TSNeurons(M = M, patterns = list())

#file <- "/home/alexeyche/prog/sim/stimuli/sd1.csv"
#file2 <- "/home/alexeyche/prog/sim/stimuli/sd2.csv"
file <- "/home/alexeyche/my/sim/stimuli/sd1.csv"
file2 <- "/home/alexeyche/my/sim/stimuli/sd2.csv"

gr1$loadPattern(file, 100, 1)
gr1$loadPattern(file2, 100, 2)
id_m = 1:M
id_n = (M+1):(M+N)
id_ro = (M+N+1):(M+N+Nro)

start_w = 5
weights = list()
id_conns = list()
ids = NULL
for(i in 1:N) {
  conn <- id_m
  conn <- c(conn, id_n[id_n != id_n[i]]) # id of srm neurons: no self connections
  w <- c(rep(start_w, M), rep(start_w/4, N-1))
  id_conns[[i]] = conn
  weights[[i]] = w
  ids = c(ids, id_n[i])
}

neurons = SRMLayer(id_conns = id_conns, weights = weights, ids=ids) 

#weights_ro = list()
#for(i in 1:Nro) {
#  weights_ro[[i]] <- c(rep(start_w, N))
#}
#ro_neurons = SMNeuronLayer(weights = weights_ro, id_conn = id_n, ids = id_ro)

layers = list(neurons)

epochs = 100
#run_mode = "run"
run_mode = "learn"
run_options = list(T0 = 0, Tmax = 100, dt = 0.5, learning_rate = 0.5,
                   learn_window_size = 100, mode=run_mode, collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_2spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1
                   )

patterns = gr1$patterns
run_net(layers, patterns, epochs, run_options)
#learn_layer(patterns, layers, run_options)