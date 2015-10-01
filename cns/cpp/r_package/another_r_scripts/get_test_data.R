
require(Rdnn)
setwd(path.expand("~/dnn/r_scripts"))
source("env.R")

fname = sprintf("%s/test_spikes.pb", SPIKES_PLACE)

num_of_patterns = 3
N = 50
patt_len = 500
gap_between_patterns = 0


patt = lapply(seq(0, patt_len, by=patt_len/N), function(x) x)

ts_out = list()
ts_out$ts_info = list()
ts_out$ts_info$unique_labels = c("1")
ts_out$ts_info$labels_ids = NULL
ts_out$ts_info$labels_timeline = NULL
ts_out$values = blank_net(N)

t0 = 0
for(i in 1:num_of_patterns) {
    for(ni in 1:N) {
        ts_out$values[[ni]] = c(ts_out$values[[ni]], patt[[ni]] + t0)
    }
    t0 = t0 + patt_len + gap_between_patterns
    ts_out$ts_info$labels_ids = c(ts_out$ts_info$labels_ids, 0)
    ts_out$ts_info$labels_timeline = c(ts_out$ts_info$labels_timeline, t0)
}


RProto$new(fname)$write(ts_out, "SpikesList")

prast(ts_out$values,T0=0,Tmax=1000)
