#!/usr/bin/env Rscript

args <- commandArgs(trailingOnly = FALSE)
if(length(grep("RStudio", args))>0) {
  verbose = TRUE
  dir='~/prog/sim/runs/testrstudio'
  #dir='~/my/sim/runs/testrstudio'
  data_dir = '~/prog/sim'
  #data_dir = '~/my/sim'
  setwd("~/prog/alexeyche-junk/cns/R/srm")
  #setwd("~/my/git/alexeyche-junk/cns/R/srm")
  #train_spikes = list("/home/alexeyche/my/sim/ucr_fb_spikes/train_spikes")
  #test_spikes  = list("/home/alexeyche/my/sim/ucr_fb_spikes/test_spikes")
  #train_spikes = list("/home/alexeyche/prog/sim/ucr_nengo_spikes/train_spikes")
  #test_spikes  = list("/home/alexeyche/prog/sim/ucr_nengo_spikes/test_spikes")
  source('constants.R')
  #model_file = "~/my/sim/runs/testrstudio/model"
  model_file = "~/prog/sim/runs/testrstudio/model"
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
  copy_ok = file.copy("make_dataset.R", dir, overwrite=TRUE)
  
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
system(sprintf("find %s -maxdepth 1 -type f -name \"*.png\" -o -name \"*.bin\" -o -name \"*.idx\" -exec rm -f {} \\;", dir))
#===================================================================================================

library(snn)

source('srm_funcs.R')
source('plot_funcs.R')
source('learn_and_run_net.R')
source('srm.R')
source('grad_funcs.R')
source('serialize_to_bin.R')
source('eval_funcs.R')
source('kernel.R')
source('eval_funcs.R')
source('ucr_ts.R')
source('gen_spikes.R')



set.seed(seed_num)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 weight_per_neuron=weight_per_neuron, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)
ID_MAX=0


gr1 = TSNeurons(M = M)
neurons = SRMLayerClass$new(N, 0, p_edge_prob=net_edge_prob,ninh=round(N*inhib_frac), syn_delay_rate, axon_delay_rate, delay_dist_gain)
neurons2 = NULL
if(N2>0) {
    neurons2 = SRMLayerClass$new(N2, 0, p_edge_prob=net_edge_prob2,ninh=round(N2*inhib_frac2), syn_delay_rate, axon_delay_rate, delay_dist_gain)
}
connection = matrix(gr1$ids(), nrow=length(gr1$ids()), ncol=N)
for(i in 1:net_neurons_for_input) {
  cc = sample(gr1$ids(), M-afferent_per_neuron)
  connection[cc,i] = 0
}

if(net_neurons_for_input<N)
  connection[,(net_neurons_for_input+1):N] = 0

neurons$connectFF(connection, 0, 1:N, syn_delay_rate, delay_dist_gain)

sl = list(neurons)
if(N2>0) {
    sl[[length(sl)+1]] = neurons2
    connection21 = matrix(neurons$ids(), nrow=N, ncol=N2)
    for(i in 1:net_neurons_for_input2) {
      cc = sample(1:N, N2-afferent_per_neuron2)
      connection21[cc,i] = 0
    }
    connection12 = matrix(neurons2$ids(), nrow=N2, ncol=N)
    for(i in 1:net_neurons_for_input) {
      cc = sample(1:N2, N-afferent_per_neuron2)
      connection12[cc,i] = 0
    }
    neurons2$connectFF(connection21, 0, 1:N2, syn_delay_rate, delay_dist_gain)
    neurons$connectFF(connection12, 0, 1:N, syn_delay_rate, delay_dist_gain)
}
for(ni in 1:N) {
    ncon = length(neurons$obj$id_conns[[ni]])
    neurons$obj$W[[ni]] = abs(rnorm(ncon, mean=weight_per_neuron/ncon, sd=start_W.sd))
}
if(N2>0) {
    for(ni in 1:N2) {
        ncon = length(neurons2$obj$id_conns[[ni]])
        neurons2$obj$W[[ni]] = rnorm(ncon, mean=weight_per_neuron2/ncon, sd=start_W.sd)
    }   
}

source('make_dataset.R')

s = SIMClass$new(sl)


for(ep in 0:epochs) {    
    if(ep == 0) { 
        work_net = train_net_mean_p
    } else {
        work_net = train_net  
    } 
    
    sim_opt = list(dt=dt, saveStat=FALSE, learn=(ep > 0), determ=FALSE)
    s$sim(sim_opt, constants, work_net)
    c(sim, discr, loss) := eval(train_net_ev, test_net_ev, s, kernel_sigma)
    pic_filename = sprintf("%s/run_ep%s.png", dir, ep)
    plot_run_status(work_net$net, neurons, sim, discr, loss, pic_filename, sprintf("Epoch %s", ep))
    cat("ep: ", ep,  "\n")
    
    model_file_ep = sprintf("%s/model_ep%s", dir, ep)
    saveModelToFile(neurons, model_file_ep)
}
