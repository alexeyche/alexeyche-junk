
verbose = TRUE
#dir='~/prog/sim/runs/test'
dir='~/my/sim/runs/test'
#data_dir = '~/prog/sim'
data_dir = '~/my/sim'
#setwd("~/prog/alexeyche-junk/cns/R/srm")
setwd("~/my/git/alexeyche-junk/cns/R/srm")
source('constants.R')
source('srm_funcs.R')

require(snn)

source('plot_funcs.R')
source('serialize_to_bin.R')


set.seed(seed_num)

ID_MAX=0

dt=1
N=10
M=100
start_w.N = 1
start_w.M = 1
ws = 0.5
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, ws4=ws^4, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)
gr1 = TSNeurons(M = M)
neurons = SRMLayerClass$new(N, start_w.N, p_edge_prob=0.0)
connection = matrix(gr1$ids(), nrow=length(gr1$ids()), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )

s = SIMClass(list(neurons))


dir2save = "~/prog/sim/rfd_files"
#dir2save = "~/my/sim/rfd_files"

T0 = 0
Tmax = 30000
T = seq(T0, Tmax, by=dt) 

Wacc = vector("list",N)

file = sprintf("%s/ep_%d_%4.1fsec", dir2save, 1, Tmax/sim_dim)
net_m = loadMatrix(file, 1)
net = list()
invisible(sapply(1:nrow(net_m), function(id) { 
    net[[id]] <<- net_m[id, which(net_m[id,] != 0)]
}))
train_net = NetSim$new(list(net), list(neurons), Tmax)

sim_opt = list(dt=dt, saveStat=FALSE, learn=TRUE, determ=FALSE)

s$sim(sim_opt, constants, net)

benchMSim = function() {
 ep =1
 file = sprintf("%s/ep_%d_%4.1fsec", dir2save, ep, Tmax/sim_dim)
 net_m = loadMatrix(file, 1)
 net = list()
 invisible(sapply(1:nrow(net_m), function(id) { 
  net[[id]] <<- net_m[id, which(net_m[id,] != 0)]
 }))
 sim_opt = list(T0=T0, Tmax=Tmax, dt=dt, saveStat=FALSE, learn=TRUE, determ=FALSE)
 s$sim(sim_opt, constants, net)
}
#SpMat per synapse
#        test replications elapsed relative user.self sys.self user.child  
#1 benchMSim()            3  37.966        1    37.266      0.5          0


# no determ 
#test replications elapsed relative user.self sys.self user.child sys.child
#1 benchMSim()            5  63.529        1    63.472    0.008          0         0
# library(rbenchmark);  benchmark(benchMSim(), replications = 5)
#


for(ep in 1:20) {
  file = sprintf("%s/ep_%d_%4.1fsec", dir2save, ep, Tmax/sim_dim)
  net_m = loadMatrix(file, 1)
  net = list()
  invisible(sapply(1:nrow(net_m), function(id) { 
   net[[id]] <<- net_m[id, which(net_m[id,] != 0)]
  }))
  
  
  sim_opt = list(T0=T0, Tmax=Tmax, dt=dt, saveStat=Tmax<5000, learn=TRUE, determ=FALSE)
  s$sim(sim_opt, constants, net)
    
  cat("ep: ", ep, "\n")
  #plotl(sapply(I_acc, mean))
#  W = get_weights_matrix(list(neurons))
#  filled.contour(W)
  if(Tmax*ep>mean_p_dur) {
    for(i in 1:N) {        
      Wacc[[i]] = cbind(Wacc[[i]], neurons$obj$W[[i]])
    }
  }
}
#c1 = list_to_matrix(neurons$obj$stat_C[[1]])[,1]
W = list_to_matrix(neurons$obj$W)
levelplot(t(W), col.regions=colorRampPalette(c("black", "white")))
#levelplot(t(Wacc[[1]]) , col.regions=colorRampPalette(c("black", "white")))
#levelplot(t(Wacc[[2]]) , col.regions=colorRampPalette(c("black", "white")))
#levelplot(t(Wacc[[3]]) , col.regions=colorRampPalette(c("black", "white")))
