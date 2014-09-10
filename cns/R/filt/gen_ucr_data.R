source('./ucr_ts.R')
source('./interpolate_ts.R')
source('./serialize_to_bin.R')

data_dir = '~/prog/sim'

sample_size = 120 

data = synth # synthetic control
#data = starlight_curves

train_fname = sprintf("%s/ts/%s/%s_TRAIN_%s", data_dir,data,data,sample_size)
test_fname = sprintf("%s/ts/%s/%s_TEST_%s", data_dir,data,data,sample_size)
if(!file.exists(sprintf("%s.bin", train_fname))) {
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
    #write.table(train_dataset_inter,file=fname,sep=" ", col.names = F, row.names = F, append=F)
    saveMatrixList(train_fname, train_dataset_inter_bin)
    saveMatrixList(sprintf("%s_labels", train_fname), list(matrix(sapply(train_dataset, function(x) x$label))) )

    #write.table(test_dataset_inter,file=fname,sep=" ", col.names = F, row.names = F)
    saveMatrixList(test_fname, test_dataset_inter_bin)
    saveMatrixList(sprintf("%s_labels", test_fname), list(matrix(sapply(test_dataset, function(x) x$label))) )
}


