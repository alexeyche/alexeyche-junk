

setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")
require(snn)
source('../tem/encode.R')

spikes_dir = "~/prog/sim/bci_spikes"
dir.create(spikes_dir, FALSE)
data_proc_dir = "~/prog/sim/bci_data_proc/"
dir.create(data_proc_dir, FALSE)
data_proc_dir_train = sprintf("%s/train", data_proc_dir)
dir.create(data_proc_dir_train, FALSE)
data_proc_dir_test = sprintf("%s/test", data_proc_dir)
dir.create(data_proc_dir_test, FALSE)

spikes_dir_train = sprintf("%s/train", spikes_dir)
dir.create(spikes_dir_train, FALSE)
spikes_dir_test = sprintf("%s/test", spikes_dir)
dir.create(spikes_dir_train, FALSE)

data_dir = "~/prog/sim/bci_data"
train_data_file = sprintf("%s/train/Competition_train_cnt.txt", data_dir)
train_lab_file = sprintf("%s/train/Competition_train_lab.txt", data_dir)
test_data_file = sprintf("%s/test/Competition_test_cnt.txt", data_dir)

nchan = 64
nsamples = 3000
nread = 10
nread_test = 10
train_data_all = scan(train_data_file, nlines=nchan*nread)
test_data_all = scan(test_data_file, nlines=nchan*nread)
train_lab_all = scan(train_lab_file, nlines=nread)
train_data = list()
test_data = list()

koeff = 0.1
for(i in 1:nread) {
    low_i = 1+(i-1)*nchan*nsamples
    high_i = low_i + nchan*nsamples - 1
    train_data_c = matrix(train_data_all[low_i:high_i], nrow=nchan, ncol=nsamples, byrow=TRUE)
    train_data[[i]] = train_data_c*koeff
}
data_file = sprintf("%s/bci_processed", data_proc_dir_train)   
saveMatrixList(data_file, train_data)

for(i in 1:nread_test) {
    low_i = 1+(i-1)*nchan*nsamples
    high_i = low_i + nchan*nsamples - 1
    test_data_c = matrix(test_data_all[low_i:high_i], nrow=nchan, ncol=nsamples, byrow=TRUE)
    test_data[[i]] = test_data_c*koeff
}
data_file = sprintf("%s/bci_processed", data_proc_dir_test)   
saveMatrixList(data_file, test_data)



# dt = 1
# t_rc = 20
# t_ref = 4
# b = 0 # mean(m[1,])
# g = 0.05
# 
# patterns = list()
# for(ni in 1:nread) {
#     m = train_data[[ni]]$data
#     net = blank_net(nchan)
#     for(chi in 1:nchan) {
#         sp = lif_encode(m[chi,]*g+b, dt, t_rc, t_ref)
#         net[[chi]] = sp
#     }
#     patterns[[ni]] = list(data=net, label=train_data[[ni]]$label)
#     #plot_rastl(net)    
# }    
#     
# test_patterns = list()
# for(ni in 1:nread_test) {
#     m = test_data[[ni]]$data
#     net = blank_net(nchan)
#     for(chi in 1:nchan) {
#         sp = lif_encode(m[chi,]*g+b, dt, t_rc, t_ref)
#         net[[chi]] = sp
#     }
#     test_patterns[[ni]] = list(data=net, label=test_data[[ni]]$label)
#     #plot_rastl(net)    
# }    
# mean_p_dur = 60000
# 
# ep = 10
# 
# 
# for(ep in 0:10) {
#     n = nread
#     if(ep==0) { 
#         n = mean_p_dur/nsamples
#     }
#     d = patterns[sample(nread, n)]
#     labs = NULL
#     net = blank_net(nchan)
#     ct = 0
#     for(i in 1:length(d)) {
#         for(chi in 1:length(d[[i]]$data)) {
#             net[[chi]] = c(net[[chi]], d[[i]]$data[[chi]] + ct)
#         }
#         ct = ct + nsamples
#         labs = c(labs, d[[i]]$label)
#     }
#     timeline = seq(nsamples, n*nsamples, by=nsamples)
#     spike_file = sprintf("%s/%s_bci", spikes_dir_train, ep)   
#     saveMatrixList(spike_file, list(list_to_matrix(net), 
#                                     matrix(timeline),
#                                     matrix(labs)
#     ))
# }
# 
# #plot_rastl(test_patterns[[5]]$data)
# #sapply(train_data, function(x) rowMeans(x$data))
