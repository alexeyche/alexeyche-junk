source('../tsne_stuff.R')
source('kernels.R')
require(rgl)

merge_lists <- function(l) {
  l_out <- list()
  it <- 1
  for(i in 1:length(l)) {
    for(j in 1:length(l[[i]])) {
      l_out[[it]] <- l[[i]][[j]]
      it <- it +1
    }
  }
  return(l_out)
}


ll <- list()
for(i in 1:10) {
  ll[[i]] <- all_list[[1]][[i]]
}
sts1 <- merge_lists(ll)
ll <- list()
for(i in 1:10) {
  ll[[i]] <- all_list[[2]][[i]]
}
sts2 <- merge_lists(ll)
ll <- list()
for(i in 1:10) {
  ll[[i]] <- all_list[[3]][[i]]
}
sts3 <- merge_lists(ll)

T <- max(max(unlist(sts1)), max(unlist(sts2)), max(unlist(sts3)))

require(kernlab)
ks <- kernelFactory("mci",T)
sts <- merge_lists(list(sts1, sts2, sts3))

KM <- computeKernelMatrix(ks, sts, c(6000,6000))
filled.contour(KM)
a <- princomp(KM)
plot3d(a$scores, col=c(rep(1,100),rep(2,100),rep(3,100)),useFreeType=FALSE)