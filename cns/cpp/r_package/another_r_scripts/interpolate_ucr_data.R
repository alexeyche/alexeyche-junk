
setwd(path.expand("~/dnn/r_scripts"))
require(Rdnn)
source('ucr_ts.R')
source('interpolate_ts.R')
source("env.R")

ts_dir = path.expand('~/dnn/datasets/ucr')

ts_dir = pj(DATASETS_PLACE, "ucr")

sample_size = 60

data = synth # synthetic control
#data = starlight_curves

normalize = function(x, min_val, max_val) {
    2*( 1 - (max_val-x)/(max_val-min_val) ) -1
}



train_fname = sprintf("%s/%s/%s_TRAIN_%s", ts_dir,data,data,sample_size)
test_fname = sprintf("%s/%s/%s_TEST_%s", ts_dir,data,data,sample_size)
if(!file.exists(train_fname)) {
    c(train_dataset, test_dataset) := read_ts_file(data, NA, ts_dir)
    train_dataset_inter = matrix(0, length(train_dataset), sample_size+1)
    test_dataset_inter = matrix(0, length(test_dataset), sample_size+1)
    max_val = max(sapply(train_dataset, function(x) max(x$data)), sapply(test_dataset, function(x) max(x$data)))
    min_val = min(sapply(train_dataset, function(x) min(x$data)), sapply(test_dataset, function(x) min(x$data)))
    for(i in 1:length(train_dataset)) {
        inter_ts = interpolate_ts(train_dataset[[i]]$data, sample_size)
        inter_ts = normalize(inter_ts, min_val, max_val)
        train_dataset_inter[i, ] = c(train_dataset[[i]]$label,inter_ts)
    }
    for(i in 1:length(test_dataset)) {
        inter_ts = interpolate_ts(test_dataset[[i]]$data, sample_size)
        inter_ts = normalize(inter_ts, min_val, max_val)
        test_dataset_inter[i, ] = c(test_dataset[[i]]$label,inter_ts)
    }
    write.table(train_dataset_inter,file=train_fname,sep=" ", col.names = F, row.names = F, append=F)
    write.table(test_dataset_inter,file=test_fname,sep=" ", col.names = F, row.names = F)
}

