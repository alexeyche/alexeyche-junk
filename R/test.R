#!/usr/bin/RScript
dyn.load('gpuext/gpuext.so')
cublasInit <- function() {
    .Call("cublas_init_R")
}    

cublasShutdown <- function() {
    .Call("cublas_shutdown_R")
}
initFromMatrix <- function(mat) {
    .Call("init_from_array_R",mat)
}
initFromMatrix(matrix(rep(0.1,5),nrow=5))
