
require(R.matlab)
require(Rdnn)

riken_dir = sprintf("%s/dnn/datasets/riken", path.expand("~"))
data_14chan = sprintf("%s/SubC_14chan_3LRR.mat", riken_dir)

data = readMat(data_14chan)

ts_info = list(
    unique_labels = as.character(labs),
    labels_ids = sapply(spl$labels, function(l) which(l == unique(spl$labels))) - 1,
    labels_timeline = spl$timeline
)




