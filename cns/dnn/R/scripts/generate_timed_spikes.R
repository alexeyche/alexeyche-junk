
require(Rdnn)
source(scripts.path("gen_poisson.R"))

sim_length = convNum(Sys.getenv('TIMED_PATTERN_LENGTH'), 60000)
test_sim_length = convNum(Sys.getenv('TIMED_PATTERN_LENGTH_TEST'), 60000)

neurons = 100
sample_gap = 250
sample_duration = 500
classes = 4
high_rate = 20

rates = sapply(1:classes, function(ci) high_rate*rbeta(neurons, 0.2,0.9))
mean_rate = rowMeans(rates)

patterns_spikes = list()
for(cl in 1:classes) {
    spikes = empty.spikes(neurons)
    
    for(i in 1:neurons) { # gen pattern
        spikes$values[[i]] = gen_poisson(1, rates[i, cl], sample_duration)[[1]]
    }
    spikes$info = list(list(start_time=0, duration=sample_duration, label=as.character(cl)))
    patterns_spikes = add.to.list(patterns_spikes, spikes)        
}

realize_patterns = function(patterns_spikes, mean_rates, sim_length, sample_gap) {
    neurons = length(patterns_spikes[[1]]$values)
    sample_durartion = patterns_spikes$info[[1]]$duration
    labs = unique(sapply(patterns_spikes, function(x) x$info[[1]]$label))
    classes = length(labs)
    
    final_spikes = empty.spikes(neurons)    
    time = 0
    while(time < sim_length) {
        for(cl in 1:classes) {
            spikes = patterns_spikes[[cl]]
            
            for(i in 1:neurons) { # gen noise
                noise_spikes = gen_poisson(1, mean_rate[i], sample_gap)[[1]]
                if(length(noise_spikes)>0) {
                    spikes$values[[i]] = c(spikes$values[[i]], sample_duration + noise_spikes)
                }
            }
            final_spikes = add.to.spikes(final_spikes, spikes, from=time)
            
            time = time + sample_duration + sample_gap
        }
    }    
    return(final_spikes)
}

final_spikes = realize_patterns(patterns_spikes, mean_rate, sim_length, sample_gap)
test_final_spikes = realize_patterns(patterns_spikes, mean_rate, test_sim_length, sample_gap)

proto.write(final_spikes, spikes.path("timed_pattern_spikes.pb"))
proto.write(test_final_spikes, spikes.path("timed_pattern_spikes_test.pb"))
