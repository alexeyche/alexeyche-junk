

args <- commandArgs(trailingOnly = FALSE)
base_dir = dirname(substring( args[grep("--file=", args)], 8))
setwd(base_dir)
param_string = substring(args[grep("--params=", args)], 10)

s = strsplit(param_string, "[|]")[[1]]

all_params = strsplit(s," ")

all_params = lapply(all_params, as.numeric)

b = all_params[[1]] # bias 
d = all_params[[2]] # tresh
kd = all_params[[3]] # capacitance
R = all_params[[4]] # resistance


dt = 1e-03

source('../serialize_to_bin.R')
source('encode.R')
require(entropy)

#dir2load = "/home/alexeyche/my/sim/ucr_fb_spikes/wavelets"
dir2load = "/home/alexeyche/prog/sim/ucr_fb_spikes/wavelets"
labels = c("train", "test")
nums = c(300, 300)

entrop_all = NULL

for(ds_num in 1:length(labels)) {
    for(ds_j in 1:nums[ds_num]) {
        m = loadMatrix( sprintf("%s/%s_wavelets", dir2load, labels[[ds_num]]), ds_j)
        entrop = NULL
        for(fi in 1:nrow(m)) {
            sp = iaf_encode(m[fi,], dt, b[fi], d[fi], 0, R[fi], kd[fi])
            if(length(sp)> 5) {
                entrop = c(entrop, entropy(diff(sp)))
            } else {
                entrop = c(entrop, 50/(1+length(sp)))
            }
        }
        entrop_all = cbind(entrop_all, entrop)
    }
}
cat(sum((rowMeans(entrop_all)^2)), "\n")
