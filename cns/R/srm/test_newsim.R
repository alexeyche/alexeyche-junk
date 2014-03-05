
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



source('util.R')
#source('gen_spikes.R')
source('plot_funcs.R')
#source('neuron.R')


ID_MAX=0
T0=0
Tmax=300000
dt=1
N=10
M=50
start_w.N = 7.5
start_w.M = 7.5
ws = 7.5

constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)

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
fun = Vectorize(function(t) {
  v = t/5
  vf = floor(v)
  if(vf %% 2  == 0) {
    fr = v - vf  
  } else {
    fr = 1 -(v - vf)   
  }
  fr_n = -1+2*fr
  return(fr_n)
})

pattern_fun = sin
pattern_frac = dt*7.5
patt_dur = 800
timeout = 400
pattern = gen_fun_pattern(M, 0, patt_dur, pattern_frac, pattern_fun)

gr1 = TSNeurons$new(M = M)
neurons = SRMLayerClass$new(N, start_w.N, p_edge_prob=0.5, ninh=N, 
                            syn_delay_rate=1, axon_delay_rate=1,delay_dist_gain=5)

connection = matrix(gr1$ids(), nrow=length(gr1$ids()), ncol=N)
input_conns = 50
for(ni in 1:N) {
    connection[sample(M, M-input_conns), ni] = 0
}
neurons$connectFF(connection, start_w.M, 1:N, syn_delay_rate=1, delay_dist_gain=5)


sl = SIMClass$new(list(neurons))
net = list()

mean_p_dur_pattern = gen_fun_pattern(M, 0, mean_p_dur, pattern_frac, pattern_fun)
sim_opt = list(T0=0, Tmax=mean_p_dur, dt=dt, saveStat=FALSE, learn=FALSE, determ=FALSE)
net[gr1$ids()] = mean_p_dur_pattern
net[neurons$ids()] = blank_net(N)

sl$sim(sim_opt, constants, net)
for(ep in 1:15) {
    net = list()
    net = blank_net(M+N)
    
    for(t0 in seq(0, Tmax-patt_dur-timeout, by=patt_dur+timeout)) {
        sapply(1:M, function(mi) {
            net[[ gr1$ids()[mi] ]] <<- c(net[[ gr1$ids()[mi] ]], pattern[[mi]]+t0)
        })
    }
    
    sim_opt = list(T0=0, Tmax=Tmax, dt=dt, saveStat=(Tmax<=5000), learn=TRUE, determ=FALSE)
    sl$sim(sim_opt, constants, net)
    cat("ep:", ep, "\n")
}

# #plot_rastl(net)
# #st = neurons$get_stat()
# #plotl(st$C[[3]][,8])
# #plotl(st$W[[3]][,1])
# 
# #plotl(st$B[3,])
 W = neurons$Wm()
 gr_pl(W)
pattern = gen_fun_pattern(M, 0, patt_dur, pattern_frac, pattern_fun)
net = list()
net = blank_net(M+N)

net[gr1$ids()] = pattern
sim_opt = list(T0=0, Tmax=patt_dur+100, dt=dt, saveStat=TRUE, learn=FALSE, determ=FALSE)

sl$sim(sim_opt, constants, net)
plot_rastl(net)