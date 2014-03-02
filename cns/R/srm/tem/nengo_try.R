
source('tem_util.R')
source('../serialize_to_bin.R')

c(trds, testds) := read_ts_file(synth, "~/prog/sim")

mm = matrix(0, nrow=300, ncol=512)
for(i in 1:300) {
  xapp = approx_ts(trds[[i]]$data, 520) # 520 ~ 512
  mm[i,] = xapp
}
write.table(mm, "/home/alexeyche/prog/sim/ts/synthetic_control/synthetic_control_TRAIN.512", sep=" ", row.names=FALSE, col.names=FALSE)

mm = matrix(0, nrow=300, ncol=512)
for(i in 1:300) {
  xapp = approx_ts(testds[[i]]$data, 520) # 520 ~ 512
  mm[i,] = xapp
}

write.table(mm, "/home/alexeyche/prog/sim/ts/synthetic_control/synthetic_control_TEST.512", sep=" ", row.names=FALSE, col.names=FALSE)

require(snn)

nengo_m_train = read.table("/home/alexeyche/prog/sim/ucr_nengo_spikes/nengo_output_train.csv", sep=",", header=FALSE)

M = 100
train_set = list()
for(i in 1:300) {
  net = blank_net(M)
  id1 = (i-1)*M+1
  id2 = (i-1)*M+M
  
  maxCol = max(sapply(1:M, function(ni) length(which(nengo_m_train[ni,]!=0))))
  mm = matrix(0, M, maxCol)
  for(ni in 1:M) {
    mm[ni,] = unlist(nengo_m_train[ni,1:maxCol])
  }
  train_set[[i]] = mm
}
saveMatrixList("/home/alexeyche/prog/sim/ucr_nengo_spikes/train_spikes", train_set)

nengo_m_test = read.table("/home/alexeyche/prog/sim/ucr_nengo_spikes/nengo_output_test.csv", sep=",", header=FALSE)
test_set = list()
for(i in 301:600) {
  net = blank_net(M)
  id1 = (i-1)*M+1
  id2 = (i-1)*M+M
  
  maxCol = max(sapply(1:M, function(ni) length(which(nengo_m_test[ni,]!=0))))
  mm = matrix(0, M, maxCol)
  for(ni in 1:M) {
    mm[ni,] = unlist(nengo_m_test[ni,1:maxCol])
  }
  test_set[[i-300]] = mm
}

saveMatrixList("/home/alexeyche/prog/sim/ucr_nengo_spikes/test_spikes", test_set)


