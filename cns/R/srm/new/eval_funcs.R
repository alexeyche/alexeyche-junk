source('kernel.R')

binKernel <- function(net_data, T0, Tmax, binSize=10) {
  maxl = max(sapply(net_data, length))
  breaks = seq(T0, Tmax, by=binSize)
  lb = length(breaks)-1
  if( (Tmax-T0)/binSize > (Tmax-T0)%/%binSize) lb = lb + 1 
  
  hist_ans = matrix(0, length(net_data), lb)
  
  if(maxl>0) {
    for(i in 1:maxl) {
      it_sp = sapply(net_data, function(sp) if(length(sp)>=i) sp[i] else -1)
      time_ids = ceiling((it_sp-T0)/binSize)
      for(j in 1:length(time_ids)) {
        if(time_ids[j]>0) {
          hist_ans[j,  time_ids[j]] = hist_ans[j,  time_ids[j]] + 1
        }
      } 
    }
  }
  return(hist_ans)
}
post_process_set2 = function(set, trials, T0, Tmax, window, sigma) {
  spikes_proc = list()
  for(id_patt in 1:(length(set)/trials)) {
    st_i = (id_patt-1)*trials+1
    fin_i = st_i + trials -1
    data_proc_l = lapply(set[st_i:fin_i], function(n) get_finger_print(n$data, T0, Tmax, window, sigma))
    data_proc = array(0, dim=c(length(set[[st_i]]$data), (Tmax-T0)/window, length(data_proc_l)))
    for(i in 1:length(data_proc_l)) {
      data_proc[,,i] = data_proc_l[[i]]
    }
    spikes_proc[[id_patt]] = list(data=apply(data_proc, c(1,2), mean), label=set[[st_i]]$label)
  }
  return(spikes_proc)
}

post_process_set = function(set, trials, T0, Tmax, kernel, kernSize) {
  spikes_proc = list()
  for(id_patt in 1:(length(set)/trials)) {
    st_i = (id_patt-1)*trials+1
    fin_i = st_i + trials -1
    data_proc_l = lapply(set[st_i:fin_i], function(n) binKernel(n$data, T0, Tmax, kernSize))
    data_proc = array(0, dim=c(length(set[[st_i]]$data), (Tmax-T0)/kernSize, length(data_proc_l)))
    for(i in 1:length(data_proc_l)) {
      data_proc[,,i] = data_proc_l[[i]]
    }
    spikes_proc[[id_patt]] = list(data=apply(data_proc, c(1,2), mean), label=set[[st_i]]$label)
  }
  return(spikes_proc)
}