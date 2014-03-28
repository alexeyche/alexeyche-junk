

ts_file_train = "/home/alexeyche/my/sim/eeg/Competition_train_cnt.txt"
nlines_train = as.numeric(system(sprintf("grep -c ^ %s", ts_file_train), intern=TRUE))
ts_train = scan(ts_file_train)
ts_train = matrix(ts_train, nrow=nlines_train, byrow=TRUE)

