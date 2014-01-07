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
source('serialize_to_bin.R')

if(!sfIsRunning()) {
  sfInit(parallel=TRUE, cpus=10)
  res = sfClusterEval(require('snnSRM'))
}

sfExport('constants')
dir = "/home/alexeyche/prog/sim/R"
system(sprintf("find %s -name \"*.png\" -type f -exec rm -f {} \\;", dir))

M = 50
N = 10
id_m = seq(1, M)
id_n = seq(M+1, M+N)

gr1 = TSNeurons(M = M, patterns = list())

file <- "/home/alexeyche/prog/sim/stimuli/sd1.csv"
file2 <- "/home/alexeyche/prog/sim/stimuli/sd2.csv"
gr1$loadPattern(file, 200)
gr1$loadPattern(file2, 200)
pattern1 <- spikeMatToSpikeList(gr1$patterns[[1]]$data)
pattern2 <- spikeMatToSpikeList(gr1$patterns[[2]]$data)
patterns = list(pattern1, pattern2)

start_w = 5.5
neurons = list()
for(i in 1:N) {
  conn <- id_m
  conn <- c(conn, id_n[id_n != id_n[i]]) # id of srm neurons: no self connections
  w <- rep(start_w, length(conn))
  neurons[[i]] = neuron(w = w, id_conn = conn, id = id_n[i])
}
null_pattern.N = list()
for(i in 1:N) {
  null_pattern.N[[i]] <- -Inf
}
net = list()
net[id_n] = null_pattern.N

epochs = 20
#run_mode = "run"
run_mode = "learn"
run_options = list(T0 = 0, Tmax = 150, dt = 0.5, learning_rate = 1, learn_window_size = 15, mode=run_mode, collect_stat=TRUE)

model_file = sprintf("%s/%dx%d_lr%3.1f_lwz_%3.1f", dir, M, N, run_options$learning_rate, run_options$learn_window_size)

if(run_mode == "run") {
   if(file.exists(paste(model_file, ".idx", sep=""))) {
       W = loadMatrix(model_file, 1)
       sapply(1:N, function(id) neurons[[id]]$w = W[,id] )
   } else {
       cat("Can't find file for model ", model.file, "\n")
   }
}


for(ep in 1:epochs) {
    for(id_patt in 1:length(patterns)) {
      net[id_m] = patterns[[id_patt]]
      c(net, neurons, sprob, spot, mean_grad) := run_srm(neurons, net, run_options)
      cat("epoch: ", ep, ", pattern # ", id_patt,"\n")
      
      W = sapply(neurons, function(n) n$w)
      not_fired = all(sapply(net[id_n], function(sp) length(sp) == 1))
      
      
      pic_filename = sprintf("%s/run_ep%s_patt%s.png", dir, ep, id_patt)
      png(pic_filename, width=1024, height=480)
      if(!not_fired)
        p1 = plot_rastl(net[id_n], sprintf("epoch %d, pattern %d", ep, id_patt))
      p2 = levelplot(W, col.regions=colorRampPalette(c("black", "white")))
      p3 = levelplot(mean_grad, col.regions=colorRampPalette(c("black", "white")))
      
      if(!not_fired)
        print(p1, position=c(0, 0, 0.5, 1), more=TRUE)
      print(p2, position=c(0.5, 0, 1, 0.5), more=TRUE)
      print(p3, position=c(0.5, 0.5, 1, 1))
      dev.off()
      system(sprintf("eog -w %s 1>/dev/null 2>/dev/null",pic_filename), ignore.stdout=TRUE, ignore.stderr=TRUE, wait=FALSE)
      net[id_n] = null_pattern.N
    }
}
W = sapply(neurons, function(n) n$w)
if(run_mode == "learn") {
    saveMatrixList(model_file, list(W))
}

