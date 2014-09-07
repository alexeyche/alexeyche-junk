setwd("~/prog/alexeyche-junk/cns/R/filt")
require(snn)

dt = 1

t_rc = 0.02 * 1000 
t_ref = 0.002 * 1000
M = 100
rate_low = 25
rate_high = 50
v_tresh = 1

duration = 512
ts_name = sprintf("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_%s", duration)
tslab_name = sprintf("%s_labels", ts_name)

labels = c(loadMatrix(tslab_name,1))
ts_num = length(labels)
ts = list()
for(i in 1:ts_num) {
    ts[[i]] = c(loadMatrix(ts_name,i))
}

min_val = min(sapply(ts, min))
max_val = max(sapply(ts, max))

for(i in 1:ts_num) {
    x_ts = ts[[i]]
    ts[[i]] =  2*(x_ts-min_val)/(max_val-min_val)-1    
}

c(gain, bias) := generate_gain_and_bias(M, -1, 1, rate_low, rate_high)
encoder = sample(c(1,-1),M, replace=TRUE)

#plot_tuning_curves(encoder, gain, bias)


patterns = list()
it = 1
for(x_ts in ts) {
    n = list(v=rep(0, M), ref=rep(0,M))
    
    spikes = NULL
    for(i in 1:length(x_ts)) {
        x = x_ts[i]
        input = x * encoder * gain + bias
        
        c(n, current_spikes) := run_neurons(input, n)
        spikes = cbind(spikes, current_spikes)
    }
    patterns[[length(patterns)+1]] = list(data=lapply(1:M, function(ni) which(spikes[ni,])), label=labels[it])
    it = it + 1
}


gap = 0

ntrain = NetClass(patterns[sample(length(patterns), length(patterns))], duration, gap=gap)

spikes_dir = "~/prog/sim/spikes/ucr"

for(ep in 1:10) {
    ntrain = NetClass(patterns[sample(length(patterns), length(patterns))], duration, gap=gap)
    saveMatrixList(sprintf("%s/%s_train_spikes",spikes_dir,ep), list(list_to_matrix(ntrain$net), 
                                                                     as.matrix(ntrain$timeline),
                                                                     as.matrix(ntrain$labels)))    
}
# ntest = NetClass(test_patterns[sample(length(test_patterns), length(test_patterns))], duration, gap=gap)
# saveMatrixList(sprintf("%s/test_spikes",spikes_dir), list(list_to_matrix(ntest$net), 
#                                                           as.matrix(ntest$timeline),
#                                                           as.matrix(ntest$labels)))    
