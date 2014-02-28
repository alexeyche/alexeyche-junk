
setwd("~/my/git/alexeyche-junk/cns/R/srm/tem")
source('../serialize_to_bin.R')
dir2load = "/home/alexeyche/my/sim/ucr_fb_spikes/wavelets"
labels = c("train", "test")
nums = c(300, 300)
net_all = list()
for(ds_num in 1:length(labels)) {
  for(ds_j in 1:nums[ds_num]) {
    m = loadMatrix( sprintf("%s/%s_wavelets", dir2load, labels[[ds_num]]), ds_j)
    net_all[[ (ds_num-1)*nums[1]+ds_j]] = m
  }
}

M_dist = matrix(Inf, nrow=length(net_all), ncol=length(net_all))

for(i in 1:length(net_all)) {
  for(j in 1:length(net_all)) {
    if(i != j) {
      M_dist[i,j] = sum((net_all[[i]] - net_all[[j]])^2)
    }
  }
}

all_ids = rep(1, 600)

for(i in 1:length(net_all)) {
  all_ids[which(M_dist[,i] < 100)] = 0
}

which(all_ids == 1)
# cat /var/tmp/all_ids | tr ' ' '\n' | sort -n | uniq | tr '\n' ','