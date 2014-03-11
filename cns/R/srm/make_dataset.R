
source('ucr_ts.R')
source('gen_spikes.R')

data = synth # synthetic control
c(train_dataset, test_dataset) := read_ts_file(data, data_dir)
elems = samples_from_dataset
train_dataset = train_dataset[ c(sample(51:101, elems))] #, sample(101:150, elems))] #, sample(101:150,elems),
#                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(101:150, elems))]  #, sample(101:150, elems),
#sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]

train_dataset[[1]]$label=1
train_dataset[[2]] = list(data = -train_dataset[[1]]$data, label=2)

train_dataset = train_dataset[sample(1:length(train_dataset))]

timeline = NULL
Tcur = 0
nr = NULL

train_net = blank_net(M)
patterns = list()
for(ds in train_dataset) {
    p = genSpikePattern(M, ds$data, duration, dt, lambda=30)
    patterns[[length(patterns)+1]] = list(data=p, label=ds$label)
    invisible(sapply(1:length(p), function(id) { 
        sp = p[[id]] + Tcur
        train_net[[id]] <<- c(train_net[[id]], sp)
    }))
    Tcur = Tcur + duration
    timeline = c(timeline, Tcur)
}
Tmax = max(sapply(train_net, function(x) if(length(x)>0) max(x) else -Inf))
