require(snn)
rundir = "/home/alexeyche/prog/sim/runs"
runname = "WtaLayer_SimpleSTDP_Exp_0002"
fname = "%s/%s/eval/postproc_%d.train.k10.0.bin"
spike_file = sprintf("%s/%s/eval_output_spikes_train.bin", rundir, runname)
classes = loadMatrix(spike_file,3)
len = length(classes)
cl_sort_ind = sort(classes, index.return=TRUE)$ix
ucl = unique(c(classes))
class_len = length(which(classes == ucl[1]))

neurons_id = 1:100

v1 = loadMatrix(sprintf(fname,rundir, runname, 1), 1)[neurons_id, ]
order_id = NULL
neuron_vals = matrix(0, nrow=length(neurons_id), ncol=ncol(v1)*len)
mi_unsorted=1
for(mi in cl_sort_ind) {
    ml = list()
    for(i in 1:10) {    
        ml[[i]] = loadMatrix(sprintf(fname,rundir,runname,i), mi)[neurons_id, ]
    }
    mmean = apply(simplify2array(ml), 1:2, mean)
    mmax = sapply(1:ncol(mmean), function(i) which.max(mmean[,i]))
    
    found_ids = mmax %in% order_id
    order_id = c(order_id, unique(mmax[!found_ids]))
    
    mmax_order_ids = which(order_id %in% mmax)
    for(i in 1:length(mmax)) {
        neuron_vals[ mmax_order_ids[i], i+(mi_unsorted-1)*ncol(v1)] <- mmean[mmax[i],i]
    }   
    mi_unsorted = mi_unsorted + 1
}
cl_i = 2
low_i = (cl_i-1)*(class_len*ncol(v1))+1
high_i = low_i + class_len*ncol(v1)-1
gr_pl(t(neuron_vals[,low_i:high_i]))
