require(kohonen)
require(snn)
setwd("~/prog/alexeyche-junk/cns/R/srm")
source('ucr_ts.R')
data = synth # synthetic control
c(train_dataset, test_dataset) := read_ts_file(data, "~/prog/sim")

rundir = "/home/alexeyche/prog/sim/runs"

runname = "n100_full"
ep=20
tr_i=3
duration=1000
Tbr=60

workdir = sprintf("%s/%s", rundir, runname)
evaldir = sprintf("%s/eval", workdir)
evalepdir = sprintf("%s/%s", evaldir, ep)
output_file = sprintf("%s/%s_output_spikes", evalepdir, tr_i)

ucr_spikes_dir = "/home/alexeyche/prog/sim/ucr_spikes_full"
input_file = sprintf("%s/train/1_ucr_50elems_6classes_1000dur", ucr_spikes_dir)
timeline = c(loadMatrix(input_file,2))
labels = c(loadMatrix(input_file,3))


train_net = getSpikesFromMatrix(loadMatrix(output_file,1))
Nids=101:200
train_resp = decomposePatterns(train_net[Nids], timeline, labels)
train_resp_k = lapply(train_resp, function(d) histKernel(d, list(Tmax=duration, Tbr=Tbr)))
data_train = t(sapply(train_resp_k, function(x) x$data))
require(tsne)
out = tsne(data_train, perplexity=5, max_iter=500)
plot(out, col=labels)

data_train_ts = t(sapply(train_dataset, function(x) x$data))
labels_ts = sapply(train_dataset, function(x) x$label)
out_ts = tsne(data_train_ts, perplexity=5, max_iter=500)
plot(out_ts,col=labels_ts)

