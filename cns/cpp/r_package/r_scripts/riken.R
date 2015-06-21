
require(R.matlab)
require(Rdnn)

riken_dir = sprintf("%s/dnn/datasets/riken", path.expand("~"))
name = "14chan_3LRR"
data_14chan = sprintf("%s/SubC_%s.mat", riken_dir, name)

norm = function(x) {
    x/sqrt(sum(x^2))
}


data = readMat(data_14chan)
d = dim(data$EEGDATA)
labels = c()
values = c()
timeline = c()
for(i in 1:2) {    
#for(i in 1:d[3]) {    
    #values = matrix(norm(data$EEGDATA[1,,1]), nrow=1)
    values = cbind(values, t(sapply(1:d[1], function(j) norm(data$EEGDATA[j,,i]))) )

    labels = c(labels, data$LABELS[i,])
    timeline = c(timeline, ncol(values))               
}


ts_info = list(
    unique_labels = as.character(unique(labels)),
    labels_ids = sapply(labels, function(l) which(l == unique(labels))) - 1,
    labels_timeline = timeline
)
dst_f = path.expand(sprintf("~/dnn/ts/riken_%s.pb", name))
pr = RProto$new(dst_f)
pr$write(list(values=values, ts_info=ts_info), "TimeSeries")



