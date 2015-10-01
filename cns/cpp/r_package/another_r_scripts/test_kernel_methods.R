
require(kernlab)
setwd("/home/alexeyche/dnn/r_scripts")
source("kernel_methods.R")


bd="/home/alexeyche/dnn/build"
setwd(bd)

m2 = as.kernelMatrix(as.matrix(read.table("out.ssv")))
labs = get.labs(m2)
m2 = centering(m2)

kpc = kpca(m2, features=2)
pcv(kpc)

plot(rotated(kpc), col=as.integer(labs))

#kf = kfa(m) #, features=2)

# #sc = specc(m, centers=4)
# #plot(sc, col=as.integer(labs))
# K = m
# m_class = NULL
# l_class = NULL
# N = 
# for(l in unique(labs)) {
#     li = which(labs == l)
#     lc = length(li)
#     class_l = c(l_class, lc)
#     Kc = K[li, li]
#     
#     lm = matrix(1/lc, nrow=lc, ncol=lc)
#     
#     one = matrix(0, nrow=lc, ncol=lc)
#     diag(one) <- 1
#     Kc %*% (one - lm) %*% t(Kc)
#     m_class = c(m_class, mean(Kc))
# }
# 
# SB = sum(class_l*(m_class - mean(m_class))^2)

