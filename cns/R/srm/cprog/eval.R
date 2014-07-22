

source('../ucr_ts.R')

rundir = "/home/alexeyche/prog/sim/spear_runs"
runname = "20053"
workdir = sprintf("%s/%s", rundir, runname)
output_file = sprintf("%s/eval_output_spikes_train",workdir)
test_output_file = sprintf("%s/eval_output_spikes_test",workdir)

train_net = getSpikesFromMatrix(loadMatrix(output_file,1))
test_net = getSpikesFromMatrix(loadMatrix(test_output_file,1))

timeline = c(loadMatrix(output_file,2))
labels = c(loadMatrix(output_file,3))

test_timeline = c(loadMatrix(test_output_file,2))
test_labels = c(loadMatrix(test_output_file,3))

Nids=101:200
train_resp = decomposePatterns(train_net[Nids], timeline, labels)
test_resp = decomposePatterns(test_net[Nids], test_timeline, test_labels)

Tbr=95
duration = 1250
train_resp_k = lapply(train_resp, function(d) histKernel(d, list(Tmax=duration, Tbr=Tbr)))
test_resp_k = lapply(test_resp, function(d) histKernel(d, list(Tmax=duration, Tbr=Tbr)))

c(r, confm_data) := ucr_test(train_resp_k, test_resp_k, eucl_dist_alg, FALSE)





