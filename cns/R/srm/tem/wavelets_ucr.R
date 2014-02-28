#!/usr/bin/RScript

source('../serialize_to_bin.R')
source('../util.R')
source('filters.R')
source('encode.R')
source('fb.R')
source('../ucr_ts.R')
source('tem_util.R')
source('../plot_funcs.R')

require(wavelets)
require(entropy)

c(trds, testds) := read_ts_file(synth, "~/my/sim")
dir2save = "/home/alexeyche/my/sim/ucr_fb_spikes/wavelets"
#dir2save = "/home/alexeyche/prog/sim/ucr_fb_spikes/wavelets"

wave_all = list()
for(ds_and_label in list(list(trds, "train"), list(testds, "test"))) {
    ds = ds_and_label[[1]]
    label = ds_and_label[[2]]
    ds_mx = list()
    for(i in 1:length(ds)) {

        xapp = approx_ts(ds[[i]]$data, 520) # 520 ~ 512

        f = wt.filter('d4', modwt=TRUE)

        nf = as.integer(log2(length(xapp)))
        mx = modwt(xapp, f, nf)
        
        ds_mx[[i]] = list_to_matrix(mx@W)
        wave_all[[length(wave_all)+1]] = mx    
    }
#    saveMatrixList(sprintf("%s/%s_wavelets", dir2save, label), ds_mx)
}


plot_dwt = function(mx) { 
    par(mfrow=c(length(mx@W),1), mar = rep(2,4))
    for(i in length(mx@W):1) {
        plot(mx@W[[i]], type="l")
    }
}

