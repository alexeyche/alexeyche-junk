

require(Rsnn)
source('./ucr_ts.R')
source('./interpolate_ts.R')

ts_dir = '~/prog/sim/ts'

sample_size = 120

data = synth # synthetic control
#data = starlight_curves

train_fname = sprintf("%s/%s/%s_TRAIN_%s", ts_dir,data,data,sample_size)
test_fname = sprintf("%s/%s/%s_TEST_%s", ts_dir,data,data,sample_size)
if(!file.exists(train_fname)) {
    c(train_dataset, test_dataset) := read_ts_file(data, NA, ts_dir)
    train_dataset_inter = matrix(0, length(train_dataset), sample_size+1)
    test_dataset_inter = matrix(0, length(test_dataset), sample_size+1)
    for(i in 1:length(train_dataset)) {
        inter_ts = interpolate_ts(train_dataset[[i]]$data, sample_size)
        train_dataset_inter[i, ] = c(train_dataset[[i]]$label,inter_ts)
    }
    for(i in 1:length(test_dataset)) {
        inter_ts = interpolate_ts(test_dataset[[i]]$data, sample_size)
        test_dataset_inter[i, ] = c(test_dataset[[i]]$label,inter_ts)
    }
    write.table(train_dataset_inter,file=train_fname,sep=" ", col.names = F, row.names = F, append=F)
    write.table(test_dataset_inter,file=test_fname,sep=" ", col.names = F, row.names = F)
}

