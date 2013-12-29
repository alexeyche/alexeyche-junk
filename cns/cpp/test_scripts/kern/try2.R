
#source('../tsne_stuff.R')
source('kernels.R')
require(rgl)

sigma <- 20

k <- Vectorize(function(t,v) {
  ss <- 0
  for(tf in v) ss <- ss + exp(-abs(t-tf)/sigma)
  return(ss)  
}, "t")


v <- all_list[[1]][[1]][[2]]
v2 <- all_list[[3]][[1]][[2]]


integrate(function(t) k(t,v)*k(t,v2), 0, 6500, subdivisions = 100000 )
