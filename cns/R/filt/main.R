
source('nengo.R')
source('util.R')
source('serialize_to_bin.R')


dt = 1

t_rc = 0.02 * 1000 
t_ref = 0.002 * 1000
M = 100
rate_low = 25
rate_high = 50
v_tresh = 1




#Xi = sample(1:10)

Xi=1:10
X = NULL
for(i in Xi) {
    X = c(X, loadMatrix("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_120",i)[1,])
}

X =  2*(X-min(X))/(max(X)-min(X))-1



c(gain, bias) := generate_gain_and_bias(M, rate_low, rate_high)
encoder = sample(c(1,-1),M, replace=TRUE)


centers = seq(-1,1,length.out=M)
delta = 0.01


n = list(v=rep(0, M), ref=rep(0,M))


spikes = blank_net(M)
for(i in 1:length(X)) {
    x = X[i]
    #input = x * encoder * gain + bias
    input = 10000*gaussFun(x, centers, delta)
    
    c(n, current_spikes) := run_neurons(input, n)
    for(ni in which(current_spikes)) {
        spikes[[ni]] = c(spikes[[ni]], i)    
    }    
}

prast(spikes,T0=0,Tmax=120)

#plot_tuning_curves(encoder, gain, bias)


