


source('../serialize_to_bin.R')
source('encode.R')
require(entropy)
source('../plot_funcs.R')
source('../snn/R/util.R')

dt = 0.1
t_rc = 2
t_ref = 0.2
Tmax = length(u)*dt


g = c(604.4949,140.6818,32.71212,17.08333,12.05808,10.04798,10.04798,8.037879,8.037879)
#g = (1 - z)/(intercept - 1.0)
b = 0

#sp = lif_encode(u*g+b, dt, t_rc, t_ref)
#par(mfrow=c(2,1))
#plot(seq(0,Tmax,length.out =length(u)), u, xlim=c(0,Tmax), type="l")
#plot(sp, rep(1,length(sp)), xlim=c(0,Tmax))

#r = length(sp)/Tmax

#dir2load = "/home/alexeyche/my/sim/ucr_fb_spikes/wavelets"
dir2load = "/home/alexeyche/prog/sim/ucr_fb_spikes/wavelets"
#dir2save = "/home/alexeyche/my/sim/ucr_fb_spikes"
dir2save = "/home/alexeyche/prog/sim/ucr_fb_spikes"
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
            sp = lif_encode(m[fi,]*g[fi]+b, dt, t_rc, t_ref)
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

plot_rastl(net_sp_all[[51]])
