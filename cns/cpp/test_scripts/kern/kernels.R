#!/usr/bin/RScript

binSpikeTrains <- function(st, T, binSize) {
    N <- floor(T/binSize)
    M <- length(st)
    x <- matrix(0, N, M)
    binEdges <- seq(0, T, by=binSize)
    binCenters <- (binEdges[1:length(binEdges)-1] + binEdges[2:length(binEdges)])/2
    for(kk in 1:M) {
        h <- hist(st[[kk]], breaks = binEdges, plot=FALSE)
        if(length(h) == 0) {
            continue
        }
        cat("N = ", N, " len(binEdges) = ", length(binEdges), "\n")
        cat("h$counts = ", h$counts, "\n")
        if(length(h$counts) != N) {
            x[,kk] = h$counts[1:length(h$counts)-1]
            x[length(binEdges), kk] = x[length(binEdges), kk] + h$counts[length(h$counts)]
        } else {
            x[,kk] = h$counts
        }            
    }
    return(x)
}
binnedKernel <- function(st1, st2, T, ksize) {
    x1 <- binSpikeTrains( list(st1), T, ksize)
    x2 <- binSpikeTrains( list(st2), T, ksize)
    v <- t(x1) %*% x2
    return(v)
}


st1 <- list(rnorm(100,mean=10, sd=1), rnorm(100, mean=9, sd=0.5))
st2 <- list(rnorm(100,mean=10, sd=1), rnorm(100, mean=9, sd=0.5))
binnedKernel(st1,st2, 20, 2)
