
require(Rdnn)

alpha = 0.2
beta = 0.8

N = 100
max_rate = 20

pattern_len = 1000

classes = 2
el_of_classes = 10

spikes_set = list()

for(cl in 1:classes) {
    rates = max_rate * rbeta(N, alpha, beta)
    for(el in 1:el_of_classes) {
        spikes = t(sapply(rates, function(r) rpois(pattern_len, r/1000.0)))
        spike_times = sapply(1:N, function(ni) which(spikes[ni, ] == 1))
        spikes_set[[length(spikes_set)+1]] = list(spikes=spike_times, label=cl)
    }
    
}

lsl = list(
    spikes_list = vector("list",N),
    labels=NULL,
    timeline=NULL
)
acc_time = 0
for( i in sample(length(spikes_set))) {
    sp_times = spikes_set[[i]]$spikes
    for(ni in 1:N) {
        lsl$spikes_list[[ni]] = c(lsl$spikes_list[[ni]], sp_times[[ni]] + acc_time)
    }
    acc_time = acc_time + pattern_len
    lsl$labels = c(lsl$labels, spikes_set[[i]]$label)
    lsl$timeline = c(lsl$timeline, acc_time)
}

#prw = RProto$new("/home/alexeyche/prog/sim/test_rate_patterns.pb")
#prw$write("LabeledSpikesList", lsl)

prast(lsl$spikes_list,T0=0,Tmax=2000)