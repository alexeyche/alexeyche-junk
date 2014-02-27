# for dt = 1e-02
b = c(0.0098687868984, 0.342274100117, 0.0295561392345, 0.418654959115, 0.117221965496, 0.0, 0.349356590009, 0.0774428562063, 0.954022777217)
d = c(1.21892249696, 0.1, 0.1, 0.392920957712, 0.1, 0.1, 0.161005882994, 0.163416471944, 0.1)
kd = c(0.1, 0.136449917762, 0.65322835574, 0.104151955315, 0.638143717035, 0.142705228061, 0.1, 0.104307072219, 0.282565337538)
R = c(0.783879343778, 0.287651006583, 0.1, 1.37094851554, 1.67950981309, 0.246658596647, 0.41030883898, 1.02869091536, 0.1)


source('../serialize_to_bin.R')
source('encode.R')
require(entropy)
source('../plot_funcs.R')

dt = 1e-02


dir2load = "/home/alexeyche/my/sim/ucr_fb_spikes/wavelets"
labels = c("train", "test")
nums = c(300, 300)

entrop_all = NULL

net_all = list()
for(ds_num in 1:length(labels)) {
    for(ds_j in 1:nums[ds_num]) {
        m = loadMatrix( sprintf("%s/%s_wavelets", dir2load, labels[[ds_num]]), ds_j)
        entrop = NULL
        net = list()
        for(fi in 1:nrow(m)) {
            sp = iaf_encode(m[fi,], dt, b[fi], d[fi], 0, R[fi], kd[fi])
            net[[fi]] = sp
            if(length(sp)> 5) {
                entrop = c(entrop, entropy(diff(sp)))
            } else {
                entrop = c(entrop, 50)
            }
        }
        entrop_all = cbind(entrop_all, entrop)
        net_all[[ds_num*ds_j]] = net
    }
}
