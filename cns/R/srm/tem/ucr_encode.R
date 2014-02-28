

b = rep(0, 9)
d = seq(0.000075, 0.025, length.out=9)
kd = rep(1, 9)
R = rep(0.1, 9)

source('../serialize_to_bin.R')
source('encode.R')
require(entropy)
source('../plot_funcs.R')
source('../snn/R/util.R')

dt = 1e-03


dir2load = "/home/alexeyche/my/sim/ucr_fb_spikes/wavelets"
dir2save = "/home/alexeyche/my/sim/ucr_fb_spikes"
labels = c("train", "test")
nums = c(300, 300)

entrop_all = NULL

net_sp_all = list()
wave_all = list()

for(ds_num in 1:length(labels)) {
    net_all = list()
    for(ds_j in 1:nums[ds_num]) {
        m = loadMatrix( sprintf("%s/%s_wavelets", dir2load, labels[[ds_num]]), ds_j)
        entrop = NULL
        net = list()
        for(fi in 1:nrow(m)) {
            sp = iaf_encode(m[fi,], dt, b[fi], d[fi], 0, kd[fi], R[fi])
            net[[fi]] = sp
            if(length(sp)> 5) {
              entrop = c(entrop, entropy(diff(sp)))
            } else {
              entrop = c(entrop, 100/(1+length(sp)))
            }
        }
        entrop_all = cbind(entrop_all, entrop)
        net_all[[ds_j]] = list_to_matrix(net)
        net_sp_all[[ (ds_num-1)*nums[1] + ds_j ]] = net
        wave_all[[ (ds_num-1)*nums[1] + ds_j ]] = m
    }
    saveMatrixList(sprintf("%s/%s_spikes", dir2save, labels[[ds_num]]), net_all)
}



filled.contour(entrop_all)
