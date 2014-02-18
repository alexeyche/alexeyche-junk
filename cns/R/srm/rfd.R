
verbose = TRUE
dir='~/prog/sim/runs/test'
#dir='~/my/sim/runs/test'
data_dir = '~/prog/sim'
#data_dir = '~/my/sim'
setwd("~/prog/alexeyche-junk/cns/R/srm")
#setwd("~/my/git/alexeyche-junk/cns/R/srm")
source('constants.R')
source('srm_funcs.R')

require(snn)
set.seed(seed_num)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, added_lrate = added_lrate, sim_dim=sim_dim)


source('plot_funcs.R')
source('serialize_to_bin.R')



ID_MAX=0

dt=1
N=10
M=100
start_w.N = 0.05
start_w.M = 0.05

gr1 = TSNeurons(M = M)
neurons = SRMLayerClass$new(N, start_w.N, p_edge_prob=0.1)
connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )

s = new(SIM)
s$addLayer(neurons$obj)

dir2save = "~/prog/sim/rfd_files"

T0 = 0
Tmax = 30000
T = seq(T0, Tmax, by=dt) 

Wacc = vector("list",N)

for(ep in 31:50) {
  file = sprintf("%s/ep_%d_%4.1fsec", dir2save, ep, Tmax/sim_dim)
  net_m = loadMatrix(file, 1)
  net = list()
  invisible(sapply(1:nrow(net_m), function(id) { 
   net[[id]] <<- net_m[id, which(net_m[id,] != 0)]
  }))
  
  
  sim_opt = list(T0=T0, Tmax=Tmax, dt=dt, saveStat=TRUE, learn=TRUE)
  s$sim(sim_opt, constants, net)
    
  cat("ep: ", ep, "\n")
  #plotl(sapply(I_acc, mean))
#  W = get_weights_matrix(list(neurons))
#  filled.contour(W)
  for(i in 1:N) {  
    Wm = list_to_matrix(neurons$obj$stat_W[[i]])  
    Wacc[[i]] = cbind(Wacc[[i]], colMeans(Wm[1:Tmax/2,]) )
    Wacc[[i]] = cbind(Wacc[[i]], colMeans(Wm[((Tmax/2)+1):Tmax,]) )
  }
}
c1 = list_to_matrix(neurons$obj$stat_C[[1]])[,1]
W = list_to_matrix(neurons$obj$W)
levelplot(t(W), col.regions=colorRampPalette(c("black", "white")))
levelplot(t(Wacc[[1]]) , col.regions=colorRampPalette(c("black", "white")))
levelplot(t(Wacc[[2]]) , col.regions=colorRampPalette(c("black", "white")))
levelplot(t(Wacc[[3]]) , col.regions=colorRampPalette(c("black", "white")))
