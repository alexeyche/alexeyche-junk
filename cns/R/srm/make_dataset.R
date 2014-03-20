
# it is include

source('ucr_ts.R')
source('gen_spikes.R')

data = synth # synthetic control
c(train_dataset, test_dataset) := read_ts_file(data, data_dir)
elems = samples_from_dataset
train_dataset = train_dataset[ c(sample(1:50, elems), sample(51:100, elems)) ] #, sample(101:150,elems),
#                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(51:100, elems))]  #, sample(101:150, elems),
#sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]

#train_dataset[[1]]$label=1
#train_dataset[[2]] = list(data = -train_dataset[[1]]$data, label=2)

patterns = list()
for(ds in train_dataset) {
    p = genSpikePattern(M, ds$data, duration/4, dt, lambda=5)
    p = lapply(p, function(sp) sp*4)
    patterns[[length(patterns)+1]] = list(data=p, label=ds$label)
}

test_patterns = list()
for(ds in test_dataset) {
    p = genSpikePattern(M, ds$data, duration/4, dt, lambda=5)
    p = lapply(p, function(sp) sp*4)
    test_patterns[[length(test_patterns)+1]] = list(data=p, label=ds$label)
}
#gr_pl(1000*sapply(patterns, function(x) sapply(x$data, length))/duration)

patterns = patterns[sample(1:length(patterns))]

train_net_ev = NetClass$new(patterns, list(neurons), duration)
test_net_ev = NetClass$new(test_patterns, list(neurons), duration)

train_net_mean_p = NetClass$new(patterns, list(neurons), duration)
train_net_mean_p$replicate(mean_p_dur)

train_net = NetClass$new(patterns, list(neurons), duration)
train_net$replicate(train_net_ev$Tmax*1)

#patterns[[1]] = list(data=lapply(seq(1,300,length.out=100), function(id) id), label=1)
#patterns[[2]] = list(data=lapply(seq(300,1,length.out=100), function(id) id), label=2)
#train_net = NetClass$new(patterns, list(neurons), duration)
#train_net$replicate(mean_p_dur)

#Tmax = max(sapply(train_net, function(x) if(length(x)>0) max(x) else -Inf))
