

require(Rsnn)

source('nengo.R')

dt = 1

t_rc = 0.02 * 1000 
t_ref = 0.002 * 1000
M = 100
rate_low = 50
rate_high = 25
v_tresh = 1



timeline = NULL

#Xi = sample(1:300)
Xi = 1:100
#Xi = 1:10


X = NULL
labels = loadMatrix("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_120_labels",1)
ulabels = unique(labels)
for(i in Xi) {    
    X = c(X, loadMatrix("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_120",i)[1,])
    timeline = c(timeline, length(X))
}

# cols = rainbow(length(ulabels))
# for(cl_i in 1:length(ulabels)) {
#     cl = ulabels[cl_i]
#     x_cl = X[ which(labels == cl), ]
#     d = density(x_cl)
#     if(cl_i == 1) { 
#         plot(d, col=cols[cl_i], type="l")
#     } else {
#         lines(d, col=cols[cl_i])
#     }
# }



X =  2*(X-min(X))/(max(X)-min(X))-1


c(gain, bias) := generate_gain_and_bias(M, rate_low, rate_high)
encoder = sample(c(1,-1),M, replace=TRUE)


centers = seq(-1,1,length.out=M)
delta = 0.01


n = list(v=rep(0, M), ref=rep(0,M))

spikes = blank_net(M)
spikes_bin = NULL
for(i in 1:length(X)) {
    x = X[i]
    input = x * encoder * gain + bias
    #input = 50*gaussFun(x, centers, delta)
    
    c(n, current_spikes) := run_neurons(input, n)    
    for(ni in  which(current_spikes)) {
        spikes[[ni]] <- c(spikes[[ni]], i)
    }
    spikes_bin = cbind(spikes_bin, as.numeric(current_spikes))
    
    if(i %in% timeline) {
        n = list(v=rep(0, M), ref=rep(0,M))
    }
}

#prast(spikes,i=2,plen=512)


#plotl(sapply(spikes, length))
#plot_tuning_curves(encoder, gain, bias)

saveMatrixList("~/prog/sim/spikes/filt_test_spikes", list(spikes_bin))
saveMatrixList("~/prog/sim/spikes/filt_test_target", list(matrix(X)))

