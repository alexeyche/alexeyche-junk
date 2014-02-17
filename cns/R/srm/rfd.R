
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
                 ws=ws, added_lrate = added_lrate)


source('util.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('neuron.R')
source('serialize_to_bin.R')



ID_MAX=0

dt=1
N=10
M=100
start_w.N = 0.1
start_w.M = 0.25

gr1 = TSNeurons(M = M)
neurons = SRMLayer(N, start_w.N, p_edge_prob=0)
connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )

W = loadMatrix("/var/tmp/rdf_weights", 1)
invisible(sapply(1:N, function(id) { 
  id_to_conn = which(W[,id] != 0)
  neurons$weights[[id]] = W[id_to_conn, id] 
  neurons$id_conns[[id]] = id_to_conn
}))

net = vector("list", M+N)
net[1:length(net)] = -Inf

gaussian_kernel = Vectorize(function(s, sigma) {
  sum((1/sqrt(2*pi*sigma^2))*exp(-(s^2)/(2*sigma^2)))
}, "s")


vmax=50
v0=1
gauss_rates = Vectorize(function(k,j) {
    ((vmax-v0)*exp(-0.01*( min( abs(j-k), 100-abs(j-k) ) )^2) + v0 )/1000
},"j")

T0 = 0
Tmax = 30000
T = seq(T0, Tmax, by=dt) 

sl = SimLayers(list(neurons))
I_acc = list()
stats = list()
Wacc = vector("list",N)
for(ep in 1:15) {
  net[1:length(net)] = -Inf
  for(curt in T) {
    if(curt %% 200 == 0) {
      mu = sample( seq(1, 100, by=1), 1)
    }
    syn_fired = (gauss_rates(mu, 1:M)*dt)>runif(M)
    for(fi in which(syn_fired==TRUE)) {
      net[[ gr1$ids[fi] ]] = c(net[[ gr1$ids[fi] ]], curt)
    }
  }
  sim_opt = list(T0=T0, Tmax=Tmax, dt=dt, saveStat=TRUE, seed=seed_num, learn=TRUE)
  s = sl$sim(sim_opt, net)
  stats[[ep]] = s$stat
  I_acc[[ep]] = s$stat$Istat
  for(ni in 1:N) {
    Wacc[[ni]] = cbind(Wacc[[ni]], neurons$weights[[ni]])
  }  
  cat("ep: ", ep, "\n")
  #plotl(sapply(I_acc, mean))
#  W = get_weights_matrix(list(neurons))
#  filled.contour(W)
  
}
W = get_weights_matrix(list(neurons))
levelplot(W, col.regions=colorRampPalette(c("black", "white")))
saveMatrixList("/var/tmp/rdf_weights", list(W))
#W = get_weights_matrix(list(neurons))
#levelplot(W, col.regions=colorRampPalette(c("black", "white")))
#saveMatrixList("/var/tmp/rfd.model", list(W))
#levelplot(t(Wacc[[2]]), col.regions=colorRampPalette(c("black", "white")))
#plotl(sapply(I_acc, mean))
#plotl(s$stat$Cstat[1,2, ])