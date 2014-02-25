
setwd("~/prog/alexeyche-junk/cns/R/srm")

require(snn)
source('constants.R')
source('serialize_to_bin.R')
source('plot_funcs.R')
source('srm_funcs.R')

ID_MAX=0

dir2save = "/home/alexeyche/prog/sim/ucr_fb_spikes"
labels = c(rep(1, 50), rep(2, 50), rep(3, 50), rep(4, 50), rep(5, 50), rep(6, 50))
file_it = 1:300
Tmax = -1
nets_train = nets_test = list()
for(j in 1:2) {
  for(i in file_it) {
    if(j==1)
      file = sprintf("%s/train_%s_cl%s", dir2save, i, labels[i])
    if(j==2)
      file = sprintf("%s/test_%s_cl%s", dir2save, i, labels[i])
    net_m = loadMatrix(file, 1)*sim_dim
    Tmax = max(Tmax, net_m)
    net = list()
    invisible(sapply(1:nrow(net_m), function(id) { 
      net[[id]] <<- net_m[id, which(net_m[id,] != 0)]
    }))
    p = list(data=net, label=labels[i])
    if(j==1)
      nets_train[[i]] = p
    if(j==2)
      nets_test[[i]] = p
  }
}
if(M != length(nets_train[[1]]$data)) {
  cat("Config is wrong. Regenerate data\n")
  q()
}

constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, added_lrate = added_lrate, sim_dim=sim_dim)



start_w.N = 0.05
start_w.M = 0.05


gr1 = TSNeurons(M = M)
neurons = SRMLayerClass$new(N, start_w.N, p_edge_prob=0.0)
connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )

s = SIMClass$new(list(neurons))
sim_opt = list(T0=0, Tmax=Tmax, dt=dt, saveStat=TRUE, learn=TRUE)
for(ep in 1:100) {
  for(patt in 1:length(nets_train)) {
    net = list()
    net[1:M] = nets_train[[patt]]$data
    net[neurons$ids()] = blank_net(N)
    
    s$sim(sim_opt, constants, net)
    plot_run_status(net, neurons, NULL, 
                    sprintf("~/prog/sim/ucr_fb/%s_%s.png", ep, patt), 
                    sprintf("epoch %s label %s", ep, nets_train[[patt]]$label)
    )
    cat("ep ", ep, " patt ", patt, "\n", sep="")
  }
}

