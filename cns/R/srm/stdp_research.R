
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
#set.seed(1234)
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, ta=ta, sim_dim=sim_dim)


source('util.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('neuron.R')

ID_MAX=0
T0=0
Tmax=200
dt=0.5


N=1
M=2

net = list()


start_w.N = 8
start_w.M = 5

gr1 = TSNeurons(M = M)
neurons = SRMLayer(N, start_w.N, p_edge_prob=1)
connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)

neurons$connectFF(connection, start_w.M, 1:N )

net = list()

n1.dwdt = n2.dwdt = NULL
epsps_sum = list()
epsps_sum[[1]] = rep(0, M)
epsps_acc = NULL
Cacc = NULL
Bacc = NULL
mean_act_acc = NULL

mean_act = 1/sim_dim
learn_window = 2
Ts = seq(T0,Tmax,by=dt)
for(tr in 1:1) {
  uu = pp = grgr = NULL
  net[[1]] = c(50)
  net[[2]] = c(90)
  net[[3]] = c(-Inf)
  
  for(ct in Ts) {
    u = neurons$u(ct, net)
    mean_act_acc = rbind(mean_act_acc, neurons$mean_act)
    uu = cbind(uu, u)
    p = probf(u)
    pp = cbind(pp, p)
    fired = ((probf(u))*dt)>runif(N)
    for(fi in which(fired==TRUE)) {
     net[[ neurons$ids[fi] ]] = c(net[[ neurons$ids[fi] ]], ct)
    }
    Cacc = rbind(Cacc, neurons$C(ct, dt, net)[[1]])
    Bacc = rbind(Bacc, neurons$B(ct, dt, net)[[1]])
    #if((ct %% learn_window == 0) && (ct>0)) {
    # gr = grad_func(neurons, ct-learn_window, ct, net, mean_act)
    # grgr = rbind(grgr, gr[[1]])
    #}
  }
  if(length(net[[3]]) == 2) {
    gr = grad_func(neurons, T0, Tmax, net, mean_act)
    n1.dwdt = rbind(n1.dwdt, c(net[[1]][1] - net[[3]][2], 0.001*gr[[1]][1]) )
    n2.dwdt = rbind(n2.dwdt, c(net[[2]][1] - net[[3]][2], 0.001*gr[[1]][2]) )
  }
}
#plot(n1.dwdt, xlim=c(-100,100))
#points(n2.dwdt, xlim=c(-100,100))

plot(Ts, Cacc[,2], type="l")
