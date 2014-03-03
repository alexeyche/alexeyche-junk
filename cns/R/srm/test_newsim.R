
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
set.seed(1234)

e0 = 30
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)


source('util.R')
#source('gen_spikes.R')
source('plot_funcs.R')
#source('neuron.R')


ID_MAX=0
T0=0
Tmax=1000
dt=0.5
N=10
M=100
start_w.N = 0.25
start_w.M = 0.25

gr1 = TSNeurons$new(M = M)
neurons = SRMLayerClass$new(N, start_w.N, p_edge_prob=1.0, ninh=N)
connection = matrix(gr1$ids(), nrow=length(gr1$ids()), ncol=N)
neurons$connectFF(connection, start_w.M, 1:N )

net = list()

gen_fun_pattern = function(M, T0, Tmax, dt, fun, frac=50) {
    net = blank_net(M)
    for(t in seq(T0, Tmax, by=dt)) { 
        val = fun(t/frac)
        val_n = (M-1)*(1+val)/2  # float 0:99
        sp_n_id = round(val_n) + 1  # int: 1:100
        net[[sp_n_id]] = c(net[[sp_n_id]], t)
    }
    return(net)
}

pattern = gen_fun_pattern(M, T0, Tmax, dt*10, sin)

net[gr1$ids()] = pattern
net[neurons$ids()] = blank_net(N)

sim_opt = list(T0=0, Tmax=Tmax, dt=dt, saveStat=TRUE, learn=TRUE)
sl = SIMClass$new(list(neurons))

sl$sim(sim_opt, constants, net)

plot_rastl(net)
#st = neurons$get_stat()
#plotl(st$C[[3]][,6])

#plotl(neurons$obj$stat_u[[2]])
