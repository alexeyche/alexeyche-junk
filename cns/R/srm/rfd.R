
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
set.seed(seed_num)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, sim_dim=sim_dim, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, mean_time=mean_time, added_lrate = added_lrate)


source('util.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('neuron.R')




ID_MAX=0
T0=0
Tmax=1000
dt=0.5
N=10
M=100
start_w.N = 0.25
start_w.M = 0.25

gr1 = TSNeurons(M = M)
neurons = SRMLayer(N, start_w.N, p_edge_prob=1)
connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )

net = vector("list", M+N)
net[1:length(net)] = -Inf

gaussian_kernel = Vectorize(function(s, sigma) {
  sum((1/sqrt(2*pi*sigma^2))*exp(-(s^2)/(2*sigma^2)))
}, "s")


get_synaptic_rates = function(mu) {
  sigma=7
  syn = 1:M - mu
  if(mu>70) {
    syn[1:30] = (syn[length(syn)]+1):(syn[length(syn)]+30)
  } else 
  if(mu<30) {
    syn[100:71] = (syn[1]-1):(syn[1]-30)
  }
  return( (890*gaussian_kernel(syn, sigma)+1)/sim_dim )
}

T0 = 0
Tmax = 1000
learn_window = 100
lrate = 0.001
T = seq(T0, Tmax, by=dt) 
uu = pp = NULL
mu=50

sl = SimLayers(list(neurons))
cumProbs = rep(0, N)
counts = 0
mean_act = NULL
mean_act_ep = rep(0, N)
mean_grads = list()
mean_Cstat = list()
for(ep in 1:100) {
  net[1:length(net)] = -Inf
  counts = 0
  cumProbs = rep(0, N)
  for(curt in T) {
    if(curt %% 200 == 0) {
      mu = sample( seq(1, 100, by=1), 1)
    }
    syn_fired = ((get_synaptic_rates(mu)))*dt>runif(M)
    for(fi in which(syn_fired==TRUE)) {
      net[[ gr1$ids[fi] ]] = c(net[[ gr1$ids[fi] ]], curt)
    }
  }
  sim_opt = list(T0=T0, Tmax=Tmax, dt=dt, saveStat=TRUE, seed=seed_num, learn=TRUE)
  #if(ep < 11) {
  #  sim_opt$learn = FALSE
  #}
  s = sl$sim(sim_opt, net)
  mean_grads[[ep]] = apply(s$stat$dwstat, c(1,2), mean)
  mean_Cstat[[ep]] = apply(s$stat$Cstat, c(1,2), mean)
  #W = get_weights_matrix(sl$l)
#     if((curt %% learn_window == 0) && (curt>0) && (!is.null(mean_act))) {
#       gr = grad_func(neurons, curt-learn_window, curt, net, mean_act)
#       
#       W = get_weights_matrix(list(neurons))
#       p3 = levelplot(t(list_to_matrix(gr)), col.regions=colorRampPalette(c("black", "white")))
#       p1 = plot_rastl(net)
#       p2 = levelplot(W, col.regions=colorRampPalette(c("black", "white")))
#       print(p1, position=c(0, 0, 1, 0.33), more=TRUE)
#       print(p2, position=c(0, 0.33, 1, 0.66), more=TRUE)
#       print(p3, position=c(0, 0.66, 1, 1))
#     
#       for(ni in 1:neurons$len) {
#         neurons$weights[[ni]] = neurons$weights[[ni]] + ratecalc(neurons$weights[[ni]])*0.01*(gr[[ni]]-neurons$weights[[ni]]*weight_decay_factor) 
#       }
#     }
#   }
#   cat("ep: ", ep, "\n")    
#   mean_act = cumProbs/counts
  
}
