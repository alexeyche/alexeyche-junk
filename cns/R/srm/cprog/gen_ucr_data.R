library(snn)
setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")

source('../ucr_ts.R')
source('../interpolate_ts.R')

data_dir = '~/prog/sim'

samples_per_class = 50

samples_from_dataset = 10
sample_size = 180

selected_classes = c(1,2,3,4)

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

train_dataset = list()
test_dataset = list()
# train_labels = c(loadMatrix(sprintf("%s_labels", train_fname),1))
# for(i in 1:length(train_labels)) {
#     train_dataset[[i]] = list(data=loadMatrix(train_fname, i), label=train_labels[i])
#     if(i>100) break
# }
# test_labels = c(loadMatrix(sprintf("%s_labels", test_fname),1))
# for(i in 1:length(test_labels)) {
#     test_dataset[[i]] = list(data=loadMatrix(test_fname, i), label=test_labels[i])
#     if(i>100) break
# }
fiter=0
for(fname in c(train_fname, test_fname)) {
    data_labels = c(loadMatrix(sprintf("%s_labels", fname), 1))
    iter=1
    data_selected = list()
    data_labels_selected = c()
    for(cl in selected_classes) {
        for(i in 1:samples_from_dataset) {    
            m = loadMatrix(fname, (cl-1)*samples_per_class+i)
            data_selected[[ length(data_selected)+1 ]] = m
            data_labels_selected = c(data_labels_selected, data_labels[ (cl-1)*samples_per_class+i ])
            if(fiter==0) {
                train_dataset[[iter]] = list(data=m, label=data_labels[ (cl-1)*samples_per_class+i ])
            } else {
                test_dataset[[iter]] = list(data=m, label=data_labels[ (cl-1)*samples_per_class+i ])
            }
            iter=iter+1
        }           
    }
    saveMatrixList(sprintf("%s_sel", fname), data_selected)
    saveMatrixList(sprintf("%s_sel_labels", fname), list(matrix(data_labels_selected)))
    fiter=fiter+1
}

source('../gen_spikes.R')    
patterns = list()
dt=1
duration=sample_size*2
M=100
for(ds in train_dataset) {
    p = genSpikePattern(M, ds$data, duration, dt, lambda=1)
    p = lapply(p, function(sp) sp*(duration/sample_size))
    patterns[[length(patterns)+1]] = list(data=p, label=ds$label)
}

test_patterns = list()
it=0
for(ds in test_dataset) {
    p = genSpikePattern(M, ds$data, duration, dt, lambda=1)
    p = lapply(p, function(sp) sp*(duration/sample_size))
    test_patterns[[length(test_patterns)+1]] = list(data=p, label=ds$label)
    cat("iter number: ", it, "\n")
    it=it+1
}

duration=duration
gap=0
spikes_dir = "~/prog/sim/spikes/ucr"
for(ep in 1:10) {
    ntrain = NetClass(patterns[sample(length(patterns), length(patterns))], duration, gap=gap)
    saveMatrixList(sprintf("%s/%s_train_spikes",spikes_dir,ep), list(list_to_matrix(ntrain$net), 
                                                                     as.matrix(ntrain$timeline),
                                                                     as.matrix(ntrain$labels)))    
}
ntest = NetClass(test_patterns[sample(length(test_patterns), length(test_patterns))], duration, gap=gap)
saveMatrixList(sprintf("%s/test_spikes",spikes_dir), list(list_to_matrix(ntest$net), 
                                                                 as.matrix(ntest$timeline),
                                                                 as.matrix(ntest$labels)))    


#c(train_dataset, test_dataset) := read_ts_file(data, sample_size,data_dir)
#elems = samples_from_dataset
#train_dataset = train_dataset[ c(sample(1:50, elems), sample(51:100, elems), sample(101:150,elems),
#                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
#test_dataset = test_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150, elems),
#                                sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]
#train_dataset_cut = list()
#test_dataset_cut = list()
#M=100
#sigma= 0.25
#gain = 5
#max_area = max(sapply(train_dataset, function(x) max(x$data)))
#min_area = min(sapply(train_dataset, function(x) min(x$data)))
#centers = seq(min_area, max_area, length.out=M)
#
#receptive_fields = function(ts, centers, sigma) {
#    ts_cut = matrix(NA, nrow=length(centers), ncol=length(ts))
#    for(ni in 1:nrow(ts_cut)) {
#        center = centers[ni]
#        ts_cut[ni, ] = gain*exp(-abs(center-ts)^2/sigma)
#    }
#    return(ts_cut)
#}
#
#for(i in 1:length(train_dataset)) {
#    ts = train_dataset[[i]]$data
#    train_dataset_cut[[i]] = receptive_fields(ts, centers, sigma)
#}
#
#for(i in 1:length(test_dataset)) {
#    ts = test_dataset[[i]]$data
#    test_dataset_cut[[i]] = receptive_fields(ts, centers, sigma)
#}
#saveMatrixList(sprintf("%s/spikes/ucr/prep_data/train", data_dir), train_dataset_cut)
#saveMatrixList(sprintf("%s/spikes/ucr/prep_data/train_labels",data_dir), list(matrix(sapply(train_dataset, function(x) x$label))))
#saveMatrixList(sprintf("%s/spikes/ucr/prep_data/test", data_dir), test_dataset_cut)
#saveMatrixList(sprintf("%s/spikes/ucr/prep_data/test_labels",data_dir), list(matrix(sapply(test_dataset, function(x) x$label))))

#gr_pl(t(ts_cut))
