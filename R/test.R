#!/usr/bin/RScript
dyn.load('gpuext/gpuext.so')
cublasInit <- function() {
    .C("cublas_init_R",result="")$result
}    

cublasShutdown <- function() {
    .C("cublas_shutdown_R",result="")$result
}
