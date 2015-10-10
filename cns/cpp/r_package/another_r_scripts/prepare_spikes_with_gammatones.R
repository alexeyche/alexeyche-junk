
require(Rdnn)

run_neurons = function(input, n) {
    dV = dt * (input - n$v)/t_rc
    n$v = n$v + dV
    n$v[ n$v<0 ] <- 0
    
    in_refr = n$ref>0
    n$v[in_refr] <- 0
    n$ref[in_refr] = n$ref[in_refr] - dt
    
    fired = n$v>v_thresh
    n$v[fired] <- 0
    n$ref[fired] <- t_ref
    
    return(list(n, fired))
}

M = 100
t_rc = 10
v_thresh = 1
t_ref = 5.0
dt = 1

low_f = 0.1
high_f = 100
samp_rate = 1000
seq.fun = seq

inp = proto.read("~/dnn/ts/synthetic_control_40_len_4_classes_train.pb")
x = inp$values[[1]]

x.c = conv.gammatones(x, seq.fun(low_f, high_f, length.out=M), samp_rate)
proto.write("~/dnn/ts/iaf_gammatones_raw.pb", list(values=x.c, ts_info=inp$ts_info), "TimeSeries")
gr_pl(t(x.c[,1:200]))

n = list(v=rep(0, M), ref=rep(0,M))
spikes = sapply(1:M, function(i) numeric(0))
spikes_bin = NULL

#x.c = x.c[, 1:2500]
for(xi in 1:ncol(x.c)) {
    c(n, current_spikes) := run_neurons(5*x.c[, xi], n)
    
    for(ni in which(current_spikes)) {
        spikes[[ni]] <- c(spikes[[ni]], xi)
    }
    spikes_bin = cbind(spikes_bin, as.numeric(current_spikes))    
}
prast(spikes,T0=0,Tmax=1000)

proto.write("~/dnn/spikes/iaf_gammatones.pb", list(values=spikes, ts_info=inp$ts_info), "SpikesList")

