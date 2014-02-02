#!/usr/bin/RScript
setwd("~/prog/alexeyche-junk/cns/R/srm/new")
#setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
require(snnSRM)
source('constants.R')
source('util.R')
source('neuron.R')
source('layers.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('grad_funcs.R')
source('llh.R')
source('srm.R')
source('target_functions.R')
ID_MAX=0

M = 50
N = 5
id_m = seq(1, M)
id_n = seq(M+1, M+N)

gr1 = TSNeurons(M = M)

file <- "/home/alexeyche/prog/sim/stimuli/sd1.csv"
#file <- "/home/alexeyche/my/sim/stimuli/sd1.csv"
gr1$loadPatternFromFile(file, 150, 1, 0.5)
#net <- spikeMatToSpikeList(gr1$patterns[[1]]$data)
net = list()
net[id_m] = gr1$patterns[[1]]$data
start_w = 5.5

neurons = SRMLayer(N, start_w)
neurons$connectFF(gr1$ids, start_w)
inh_id = neurons$ids[1]

for(i in 1:N) {
    dd = which(neurons$id_conns[[i]] == inh_id)
    neurons$weights[[i]][dd] = -start_w
}

null_pattern = list()
for(i in 1:N) {
  null_pattern[[i]] <- -Inf
}
pattern = list()
pattern[[1]] <- c(-Inf, 5) #, 45, 60 , 75, 95)
pattern[[2]] <- c(-Inf, 65)
pattern[[3]] <- c(-Inf, 50,80)
pattern[[4]] <- c(-Inf, 100)
pattern[[5]] <- c(-Inf, 10)

epochs = 100
run_options = list(T0 = 0, Tmax = 150, dt = 0.5, learning_rate = 0.25, learn_window_size = 150, mode="run", collect_stat=FALSE)
layers = SimLayers(list(neurons))
target_set = list(target_function_gen = full_spike_tf, depress_null=TRUE)

net_all = list()
grads = list()

for(ep in 1:epochs) {
  net[id_n] <- pattern
  #gr = grad_func(layers[[1]], 0, 150, net, target_set)
  gr = layers$l[[1]]$grad(0, 150, net, target_set)
  net[id_n] <- null_pattern
  c(net, layers, stat) := run_srm(layers, net, run_options)
  
  not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
  
 if(!not_fired)
   p1 = plot_rastl(net[id_n], sprintf("epoch %d", ep))
  
 p2 = levelplot(sapply(gr, function(x) x), col.regions=colorRampPalette(c("black", "white")))
 if(!not_fired) 
   print(p1, position=c(0, 0.5, 1, 1), more=TRUE)
 print(p2, position=c(0, 0, 1, 0.5))
  net_all[[ep]] = net[id_n]
  grads[[ep]] = gr
  
  invisible(sapply(1:N, function(i) layers$l[[1]]$weights[[i]] <- layers$l[[1]]$weights[[i]] + run_options$learning_rate * gr[[i]] ))
  
}


