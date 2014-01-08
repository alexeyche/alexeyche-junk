#!/usr/bin/RScript
setwd("~/prog/alexeyche-junk/cns/R/srm/new")

source('include.R')

if(!sfIsRunning()) {
  sfInit(parallel=TRUE, cpus=10)
  res = sfClusterEval(require('snnSRM'))
}

sfExport('constants')
dir = "/home/alexeyche/prog/sim/R"
system(sprintf("find %s -name \"*.png\" -type f -exec rm -f {} \\;", dir))

M = 50
N = 10

gr1 = TSNeurons(M = M, patterns = list())

file <- "/home/alexeyche/prog/sim/stimuli/sd1.csv"
file2 <- "/home/alexeyche/prog/sim/stimuli/sd2.csv"
gr1$loadPattern(file, 100, 1)
gr1$loadPattern(file2, 100, 2)
id_m = 1:M
id_n = (M+1):(M+N)
start_w = 7
neurons = list()
for(i in 1:N) {
  conn <- id_m
  conn <- c(conn, id_n[id_n != id_n[i]]) # id of srm neurons: no self connections
  w <- c(rep(start_w, M), rep(start_w/4, N-1))
  neurons[[i]] = neuron(w = w, id_conn = conn, id = id_n[i])
}
Nro = 2
id_ro = seq(M+N+1,M+N+Nro)
for(i in 1:Nro) {
  conn <- id_n
  w <- c(rep(start_w, N))
  neurons[[N+i]] = neuron(w = w, id_conn = conn, id = id_ro[i])
}

epochs = 50
run_mode = "run"
#run_mode = "learn"
run_options = list(T0 = 0, Tmax = 100, dt = 0.5, learning_rate = 3, 
                   learn_window_size = 20, mode=run_mode, collect_stat=TRUE, 
                   learn_neurons=id_ro,
                   target_function_gen = function(nspikes, class) {  
                     function(nspike_id) { 
                       if(class == nspike_id) {
                         return(nspikes[[nspike_id]])
                       } else {
                         return(NULL)
                       }
                     } 
                   })

run_net(neurons, gr1$patterns, epochs, run_options)
