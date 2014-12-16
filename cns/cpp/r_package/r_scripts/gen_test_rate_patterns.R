
require(Rsnn)

alpha = 0.2
beta = 0.8

N = 100
max_rate = 50

pattern_len = 1000

classes = 2
el_of_classes = 10

lsl = list(
    spikes_list = vector("list",N),
    labels=NULL,
    timeline=NULL
)


acc_time = 0
for(cl in 1:classes) {
    rates = max_rate * rbeta(N, alpha, beta)
    for(el in 1:el_of_classes) {
        spikes = t(sapply(rates, function(r) rpois(pattern_len, r/1000.0)))
        spike_times = sapply(1:N, function(ni) which(spikes[ni, ] == 1))
        for(ni in 1:N) {
            lsl$spikes_list[[ni]] = c(lsl$spikes_list[[ni]], spike_times[[ni]] + acc_time)
        }
        acc_time = acc_time + pattern_len
        lsl$labels = c(lsl$labels, as.character(cl))
        lsl$timeline = c(lsl$timeline, acc_time)
    }
    
}

prw = RProto$new("/home/alexeyche/prog/sim/test_rate_patterns.pb")
prw$write("LabeledSpikesList", lsl)

