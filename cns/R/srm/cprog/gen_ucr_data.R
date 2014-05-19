library(snn)
setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")

source('../ucr_ts.R')
source('../interpolate_ts.R')

data_dir = '~/prog/sim'


samples_from_dataset = 25
sample_size = 1000

data = synth # synthetic control
if(!file.exists(sprintf("%s/ts/%s/%s_TRAIN_%s", data_dir,data,data,sample_size))) {
    c(train_dataset, test_dataset) := read_ts_file(data, NA, data_dir)
    train_dataset_inter = matrix(0, length(train_dataset), sample_size+1)
    test_dataset_inter = matrix(0, length(test_dataset), sample_size+1)
    train_dataset_inter_bin = list()
    test_dataset_inter_bin = list()
    for(i in 1:length(train_dataset)) {
        inter_ts = interpolate_ts(train_dataset[[i]]$data, sample_size)
        train_dataset_inter[i, ] = c(train_dataset[[i]]$label,inter_ts)
        train_dataset_inter_bin[[i]] = matrix(inter_ts, nrow=1, ncol=length(inter_ts))
    }
    for(i in 1:length(test_dataset)) {
        inter_ts = interpolate_ts(test_dataset[[i]]$data, sample_size)
        test_dataset_inter[i, ] = c(test_dataset[[i]]$label,inter_ts)
        test_dataset_inter_bin[[i]] = matrix(inter_ts, nrow=1, ncol=length(inter_ts))
    }
    fname = sprintf("%s/ts/%s/%s_TRAIN_%s", data_dir,data,data,sample_size)
    write.table(train_dataset_inter,file=fname,sep=" ", col.names = F, row.names = F, append=F)
    saveMatrixList(fname, train_dataset_inter_bin)
    saveMatrixList(sprintf("%s_labels", fname), list(matrix(sapply(train_dataset, function(x) x$label))) )

    fname = sprintf("%s/ts/%s/%s_TEST_%s", data_dir,data,data,sample_size)
    write.table(test_dataset_inter,file=fname,sep=" ", col.names = F, row.names = F)
    saveMatrixList(fname, test_dataset_inter_bin)
    saveMatrixList(sprintf("%s_labels", fname), list(matrix(sapply(test_dataset, function(x) x$label))) )
}

c(train_dataset, test_dataset) := read_ts_file(data, sample_size,data_dir)
elems = samples_from_dataset
train_dataset = train_dataset[ c(sample(1:50, elems), sample(51:100, elems), sample(101:150,elems),
                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150, elems),
                                sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]
train_dataset_cut = list()
test_dataset_cut = list()
M=100
sigma= 0.25
gain = 5
max_area = max(sapply(train_dataset, function(x) max(x$data)))
min_area = min(sapply(train_dataset, function(x) min(x$data)))
centers = seq(min_area, max_area, length.out=M)

receptive_fields = function(ts, centers, sigma) {
    ts_cut = matrix(NA, nrow=length(centers), ncol=length(ts))
    for(ni in 1:nrow(ts_cut)) {
        center = centers[ni]
        ts_cut[ni, ] = gain*exp(-abs(center-ts)^2/sigma)
    }
    return(ts_cut)
}

for(i in 1:length(train_dataset)) {
    ts = train_dataset[[i]]$data
    train_dataset_cut[[i]] = receptive_fields(ts, centers, sigma)
}

for(i in 1:length(test_dataset)) {
    ts = test_dataset[[i]]$data
    test_dataset_cut[[i]] = receptive_fields(ts, centers, sigma)
}
saveMatrixList(sprintf("%s/spikes/ucr/prep_data/train", data_dir), train_dataset_cut)
saveMatrixList(sprintf("%s/spikes/ucr/prep_data/train_labels",data_dir), list(matrix(sapply(train_dataset, function(x) x$label))))
saveMatrixList(sprintf("%s/spikes/ucr/prep_data/test", data_dir), test_dataset_cut)
saveMatrixList(sprintf("%s/spikes/ucr/prep_data/test_labels",data_dir), list(matrix(sapply(test_dataset, function(x) x$label))))

#gr_pl(t(ts_cut))
