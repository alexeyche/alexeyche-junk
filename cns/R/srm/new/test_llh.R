#!/usr/bin/RScript
setwd("~/prog/alexeyche-junk/cns/R/srm/new")
require(snnSRM)
require(snowfall)
source('util.R')
source('neuron.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('grad_funcs.R')
source('llh.R')
source('srm.R')

if(!sfIsRunning()) {
  sfInit(parallel=TRUE, cpus=10)
  res = sfClusterEval(require('snnSRM'))
}
sfExport('constants')
dir = "/home/alexeyche/prog/sim/R"

M = 50
N = 5
id_m = seq(1, M)
id_n = seq(M+1, M+N)

gr1 = TSNeurons(M = M, patterns = list())

file <- "/home/alexeyche/prog/sim/stimuli/sd1.csv"
gr1$loadPattern(file, 150)
net <- spikeMatToSpikeList(gr1$patterns[[1]]$data)


start_w = 5.5

neurons = list()
for(i in 1:N) {
  conn <- id_m
  conn <- c(conn, id_n[id_n != id_n[i]]) # id of srm neurons: no self connections
  w <- rep(start_w, length(conn))
  neurons[[i]] = neuron(w = w, id_conn = conn, id = id_n[i])
}
null_pattern = list()
for(i in 1:N) {
  null_pattern[[i]] <- -Inf
}
pattern = list()
pattern[[1]] <- c(-Inf, 75)
pattern[[2]] <- c(-Inf, 65)
pattern[[3]] <- c(-Inf, 50,80)
pattern[[4]] <- c(-Inf, 100)
pattern[[5]] <- c(-Inf, 10)

epochs = 50
run_options = list(T0 = 0, Tmax = 150, dt = 0.5, learning_rate = 3, learn_window_size = 10, mode="run", collect_stat=FALSE)

for(ep in 1:epochs) {
  net[id_n] <- pattern
  gr = grad(neurons, 0, 150, net, FALSE)
  net[id_n] <- null_pattern
  c(net, neurons, sprob, spot) := run_srm(neurons, net, run_options)
  not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
  
  if(!not_fired)
    p1 = plot_rastl(net[id_n], sprintf("epoch %d", ep))
  p2 = levelplot(gr, col.regions=colorRampPalette(c("black", "white")))
  if(!not_fired) 
    print(p1, position=c(0, 0.5, 1, 1), more=TRUE)
  print(p2, position=c(0, 0, 1, 0.5))
  
  invisible(sapply(1:N, function(i) neurons[[i]]$w <- neurons[[i]]$w + run_options$learning_rate * gr[,i] ))
  
}


