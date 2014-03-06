#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = FALSE)
if(length(grep("RStudio", args))>0) {
  verbose = TRUE
  #dir='~/prog/sim/runs/test'
  dir='~/my/sim/runs/testrstudio'
  #data_dir = '~/prog/sim'
  data_dir = '~/my/sim'
  #setwd("~/prog/alexeyche-junk/cns/R/srm")
  setwd("~/my/git/alexeyche-junk/cns/R/srm")
  #train_spikes = list("/home/alexeyche/my/sim/ucr_fb_spikes/train_spikes")
  #test_spikes  = list("/home/alexeyche/my/sim/ucr_fb_spikes/test_spikes")
  #train_spikes = list("/home/alexeyche/prog/sim/ucr_nengo_spikes/train_spikes")
  #test_spikes  = list("/home/alexeyche/prog/sim/ucr_nengo_spikes/test_spikes")
  source('constants.R')
} else {
  if(length(args) == 5) {
    cat("Available options: \n")
    cat("\t\t--const-file=%constants.R% constants.R by default\n")
    cat("\t\t--runs-dir=%dir_with_runs% created dir for runs\n")
    cat("\t\t--run-name=%name% name for current run (data by default)\n")
    cat("\t\t--spike-dir=%dir_with_spikes%\n")
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
  copy_ok = file.copy(const_file, dir, overwrite=TRUE)
  
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

  model_file = substring(args[grep("--model-file=", args)], 14)
  if(length(model_file) == 0) {
    model_file = NULL
  }
  spike_dir = substring(args[grep("--spike-dir=", args)], 13)
  train_spikes = list()
  test_spikes = list()
  if(length(spike_dir) != 0) {
    spike_files = system(sprintf("find %s -maxdepth 1 -name *.bin", spike_dir), intern=TRUE)
    spike_files = sapply(strsplit(spike_files, "[.]"), function(x) x[1])
    if(length(spike_files) == 0) {
        cat("Need *.bin files with spikes in spike dir\n")
        q()
    }
    for(f in spike_files) {
        if(length(grep("train", f)) > 0) {
            train_spikes[[ length(train_spikes)+1 ]] = f
        } else
        if(length(grep("test", f)) > 0) {
            train_spikes[[ length(train_spikes)+1 ]] = f
        } else {
            cat("Need 'train' or 'test' pattern in spike files\n")
            q()
        }
    }
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
source('gen_spikes.R')



set.seed(seed_num)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, ws4=ws^4, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)
ID_MAX=0

data = synth # synthetic control
c(train_dataset, test_dataset) := read_ts_file(data, data_dir)
elems = samples_from_dataset
train_dataset = train_dataset[ c(sample(51:101, elems))] #, sample(101:150, elems))] #, sample(101:150,elems),
#                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(101:150, elems))]  #, sample(101:150, elems),
                              #sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]

train_dataset[[1]]$label=1
train_dataset[[2]] = list(data = -train_dataset[[1]]$data, label=2)

train_dataset = train_dataset[sample(1:length(train_dataset))]



labels = sapply(train_dataset, function(x) x$label)


timeline = NULL

Tcur = 0
nr = NULL

train_net = blank_net(M)
for(ds in train_dataset) {
    p = genSpikePattern(M, ds$data, duration, dt, lambda=30)
    
    invisible(sapply(1:length(p), function(id) { 
          sp = p[[id]] + Tcur
          train_net[[id]] <<- c(train_net[[id]], sp)
    }))
    Tcur = Tcur + duration
    timeline = c(timeline, Tcur)
}
Tmax = max(sapply(train_net, function(x) if(length(x)>0) max(x) else -Inf))

start_w.M = matrix(rnorm( M*N, mean=start_w.M.mean, sd=start_w.M.sd), ncol=N, nrow=M)
start_w.N = matrix(rnorm( (N-1)*N, mean=start_w.N.mean, sd=start_w.N.sd), ncol=N, nrow=(N-1))

gr1 = TSNeurons(M = M)
neurons = SRMLayerClass$new(N, start_w.N, p_edge_prob=net_edge_prob,ninh=round(N*inhib_frac))
connection = matrix(gr1$ids(), nrow=length(gr1$ids()), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )

s = SIMClass$new(list(neurons))
# collect statistics

loss = NULL

for(ep in 0:epochs) {
    net = list()
    net[neurons$ids()] = blank_net(N)
    nruns = 0
    for(T0 in seq(0, mean_p_dur/6-Tmax, by=Tmax)) {
        for(i in gr1$ids()) {
            net[[i]] = c(net[[i]], train_net[[i]]+T0 )
        }
        nruns = nruns + 1
    } 
    
    
    sim_opt = list(T0=0, Tmax=max(sapply(net, function(x) if(length(x)>0) max(x) else -Inf)), 
                   dt=dt, saveStat=FALSE, learn=(ep > 0), determ=FALSE, patternTimeline = timeline)
    s$sim(sim_opt, constants, net)
    pic_filename = sprintf("%s/run_ep%s.png", dir, ep)
    plot_run_status(net, neurons, loss, rep(timeline, nruns), rep(labels, nruns), pic_filename, sprintf("Epoch %s", ep))
    cat("ep: ", ep, "\n")    
}

#W = list_to_matrix(neurons$obj$W)
#gr_pl(W)
#levelplot(t(Wacc[[1]]) , col.regions=colorRampPalette(c("black", "white")))

W = neurons$Wm()
if(runmode == "learn") {
    saveMatrixList(model_file, list(W))
}
cat(min(loss), "\n")

