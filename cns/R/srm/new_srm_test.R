

require(snn)

source('constants.R')

constants = list(dt=dt, e0=e0, ts=ts, tm=tm, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest, pr=pr, gain_factor=gain_factor, 
                 ta=ta, tc=tc,
                 target_rate=target_rate,
                 target_rate_factor=target_rate_factor,
                 weight_decay_factor=weight_decay_factor,
                 ws=ws, added_lrate = added_lrate, sim_dim=sim_dim, mean_p_dur=mean_p_dur)


mp = NULL
dt=1
inp_rates = seq(0, 50, by=1)
for(inp_rate in inp_rates) {
    ID_MAX=0
    neuron = SRMLayerClass$new(1, 0.4, 0)
    neuron$connectFF(1:100, 0.4)
    neuron$obj$ids[1] = 101 
    s = SIMClass$new(list(neuron))
    net <- list() 
    net <- blank_net(101)
    for(t in seq(0, 10000, by=dt)) {
      fired = ((inp_rate/1000)*dt)>runif(100)  
      for(fi in which(fired == TRUE)) {
        net[[fi]] = c(net[[fi]], t)
      }
    }    
    s$sim(list(T0=0,Tmax=10000, dt=1, saveStat=TRUE, learn=FALSE), constants, net) 
    pp = neuron$obj$stat_p[[1]]
    mp = c(mp, mean(pp))
  
#    plot(density(diff(net[[101]][net[[101]]>0])))
    cat("inp_rate: ", inp_rate, " mean_rate: ", mean(pp)*1000 ,"\n")
}
