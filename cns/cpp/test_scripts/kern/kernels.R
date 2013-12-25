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
    x <- binSpikeTrains( list(st1, st2), T, ksize)
    v <- x[,1] %*% t(x[,2])
    return(v)
}

pairwiseL1 <- function(x, y) {
   xm <- matrix(x, nrow=length(x),ncol=length(y))
   abs(sweep(xm, 2, y))
}

mci <- function(st1, st2, ksize) {
    inside = pairwiseL1(st1,st2)
    sum(exp(-c(inside)/ksize))
}
dmci <- function(st1, st2, ksize) {
    inside = pairwiseL1(st1,st2)
    sum(c(inside) * exp(c(-inside)/ksize))/ksize^2
}   

nci <- function(st1, st2, 


st1 <- rnorm(100,mean=10, sd=1)
st2 <- rnorm(100, mean=9, sd=0.5)
binnedKernel(st1,st2, 20, 2)
