require(Rsnn)
source('./ucr_ts.R')

ts_dir = '~/prog/sim/ts'

sample_size = 512

data = synth # synthetic control
#data = starlight_curves

train_fname = sprintf("%s/%s/%s_TRAIN_%s_fft", ts_dir,data,data,sample_size)
test_fname = sprintf("%s/%s/%s_TEST_%s_fft", ts_dir,data,data,sample_size)
if(!file.exists(train_fname)) {
    c(train_dataset, test_dataset) := read_ts_file(data, sample_size,ts_dir)
    train_dataset_fft = matrix(0, length(train_dataset), sample_size+1)
    test_dataset_fft = matrix(0, length(test_dataset), sample_size+1)
    for(i in 1:length(train_dataset)) {
        f_data = Re(fft(train_dataset[[i]]$data))
        train_dataset_fft[i, ] = c(train_dataset[[i]]$label, f_data)
    }
    for(i in 1:length(test_dataset)) {
        f_data = Re(fft(test_dataset[[i]]$data))
        test_dataset_fft[i, ] = c(test_dataset[[i]]$label, f_data)
    }
    write.table(train_dataset_fft,file=train_fname,sep=" ", col.names = F, row.names = F, append=F)
    write.table(test_dataset_fft,file=test_fname,sep=" ", col.names = F, row.names = F)
}




