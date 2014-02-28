

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

dir2load = "/home/alexeyche/my/sim/ucr_fb_spikes/wavelets"
#dir2load = "/home/alexeyche/prog/sim/ucr_fb_spikes/wavelets"
labels = c("train", "test")
nums = c(300, 300)

entrop_all = NULL
good_ids = c(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,84,92,98,101,103,106,109,110,114,118,121,124,126,128,132,136,137,138,140,145,146,149,150,152,153,155,159,164,165,166,167,170,171,173,175,176,178,179,181,187,188,189,190,192,194,198,199,201,202,203,206,208,210,212,215,216,218,219,220,222,223,224,226,229,230,231,232,233,234,235,236,240,243,245,246,247,248,250,252,253,254,256,259,260,261,262,265,266,267,269,271,273,276,278,279,280,281,282,285,288,289,290,291,292,293,295,297,300,301,302,303,304,305,306,307,308,309,310,311,312,313,314,315,316,317,318,319,320,321,322,323,324,325,326,327,328,329,330,331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,353,360,366,381,395,401,403,404,406,407,411,412,418,421,424,425,426,428,429,430,433,434,435,436,437,438,440,441,442,443,445,446,447,448,449,451,453,459,462,464,465,466,467,473,477,481,482,483,488,495,496,497,498,501,508,510,514,515,516,518,519,520,521,522,523,526,527,530,531,533,534,536,537,538,539,540,543,546,547,548,549,550,552,554,555,556,557,559,560,562,564,565,566,568,570,573,574,575,577,578,579,583,588,593,594,596,597,598,599)

for(ds_j in 1:length(good_ids)) {
  lab = "train"
  id = good_ids[ds_j]
  if(good_ids[ds_j] >300) {
    lab = "test"
    id = id - 300
  }
  m = loadMatrix( sprintf("%s/%s_wavelets", dir2load, lab), id )
  entrop = NULL
  for(fi in 1:nrow(m)) {
    sp = iaf_encode(m[fi,], dt, b[fi], d[fi], 0, R[fi], kd[fi])
    if(length(sp)> 5) {
      entrop = c(entrop, entropy(diff(sp)))
    } else {
      entrop = c(entrop, 100/(1+length(sp)))
    }
  }
  entrop_all = cbind(entrop_all, entrop)
}
cat(sum((rowMeans(entrop_all)^2)), "\n")
