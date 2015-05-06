
require(Rdnn)
setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")
source("ucr_ts.R")

ts_dir = "~/prog/ts"
sample_size = 60
data_name = synth
c(data_train, data_test) := read_ts_file(data_name, sample_size, ts_dir)

min_train = min(sapply(data_train, function(x) min(x$data)))
min_test = min(sapply(data_test, function(x) min(x$data)))
min_val = min(min_train, min_test)

max_train = max(sapply(data_train, function(x) max(x$data)))
max_test = max(sapply(data_test, function(x) max(x$data)))
max_val = max(max_train, max_test)

N = 100
dt = 5
gap_between_patterns = 100

intercept = seq(min_val, max_val, length.out=N)

#sel = c(1:10)
#sel = c(1:10, 51:60, 101:110, 151:160)
#sel = c(1:length(data_train))
sel = c(151:200)
data_complect = list(train=data_train, test=data_test)
spikes_complect = list()

for(data_part in names(data_complect)) {
    ts = data_complect[[data_part]]
    
    sp = list(timeline = NULL, labels = NULL, dt=dt, gap_between_patterns=gap_between_patterns)
    
    sp$spikes_list = lapply(1:N, function(i) x <- vector(mode="numeric", length=0))
    
    time = 0
    for(i in sel) {
        for(x in ts[[i]]$data) {
            d = abs(x - intercept)
            ni = which(d == min(d))
            sp$spikes_list[[ni]] = c(sp$spikes_list[[ni]], time)
            time = time + dt
        }
        time = time + gap_between_patterns
        sp$timeline = c(sp$timeline, time)
        sp$labels = c(sp$labels, ts[[i]]$label)
    }
    
    spikes_complect[[data_part]] = sp
}

dst_dir= "/home/alexeyche/dnn/spikes"
ts_dst_dir= "/home/alexeyche/dnn/ts"
dir.create(dst_dir, FALSE, TRUE)
dir.create(ts_dst_dir, FALSE, TRUE)
for(data_part in names(spikes_complect)) {
    spl = spikes_complect[[data_part]]
    labs = unique(spl$labels)
    ts_info = list(
        unique_labels = as.character(labs),
        labels_ids = sapply(spl$labels, function(l) which(l == unique(spl$labels))) - 1,
        labels_timeline = spl$timeline
    )
    out = list(
        ts_info = ts_info,
        values = spl$spikes_list
    )   
    fname = sprintf("%s/%s_%s_len_%s_classes_%s.pb", dst_dir, data_name, length(sel), length(labs), data_part)
    RProto$new(fname)$write(out, "SpikesList")
    spikes_complect[[data_part]] = out
    ts_out = list(
        ts_info = ts_info,
        values = unlist(lapply(data_complect[[data_part]], function(x) x$data))
    )
    fname = sprintf("%s/%s_%s_len_%s_classes_%s.pb", ts_dst_dir, data_name, length(sel), length(labs), data_part)
    RProto$new(fname)$write(ts_out, "TimeSeries")
}
prast(spikes_complect[["train"]]$values,T0=0,Tmax=1000)
