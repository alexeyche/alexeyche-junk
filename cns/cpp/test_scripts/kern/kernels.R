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
        #cat("N = ", N, " len(binEdges) = ", length(binEdges), "\n")
        #cat("h$counts = ", h$counts, "\n")
        if(length(h$counts) != N) {
            x[,kk] = h$counts[1:length(h$counts)-1]
            x[length(binEdges), kk] = x[length(binEdges), kk] + h$counts[length(h$counts)]
        } else {
            x[,kk] = h$counts
        }            
    }
    return(x)
}
binnedLinear <- function(ks, st1, st2, ksize) {
    x <- binSpikeTrains( list(st1, st2), ks$T, ksize[1])
    v <- t(x[,1]) %*%  x[,2]
    return(v)
}

pairwiseL1 <- function(x, y) {
   xm <- matrix(x, nrow=length(x),ncol=length(y))
   abs(sweep(xm, 2, y))
}

mci <- function(ks, st1, st2, ksize) {
    inside = pairwiseL1(st1,st2)
    sum(exp(-c(inside)/ksize[1]))
}
dmci <- function(ks, st1, st2, ksize) {
    inside = pairwiseL1(st1,st2)
    sum(c(inside) * exp(c(-inside)/ksize[1]))/ksize[1]^2
}   

nci <- function(ks, st1, st2, ksize) {
    m1 <- mci(ks, st1, st1, ksize[1])
    m2 <- mci(ks, st1, st2, ksize[1])
    m3 <- mci(ks, st2, st2, ksize[1])
    D2 <- m1 + m3 - 2*m2
    return(ks$K( sqrt(D2), ksize[2]))
}

nci_autoInner <- function(st1, st2, ksize) {
    m1 <- mci(ks, st1, st1, ksize[1])
    m2 <- mci(ks, st1, st2, ksize[1])
    m3 <- mci(ks, st2, st2, ksize[1])
    return(sqrt(abs(m1 + m3 - 2*m2)))
}

scalarKernelFactory <- function(kern_str) {
    k <- NULL
    if(kern_str == "gaussian") {
        k <- function(z, ksize) exp(-(z^2)/(2*ksize^2))
    } else {
        print("error. can't find ", kern_str)
    }
    return(k)
}

computeKernelMatrix <- function(ks, sts, ksize, sts2=NULL) {
    N <- length(sts)
    if(is.null(sts2)) {
        KM <- matrix(0, nrow=N, ncol=N) 
        for(k1 in 1:N) {
            for(k2 in k1:N) {
                if ((length(sts[[k1]]) == 0)||(length(sts[[k2]]) == 0)) {
                  KM[k1, k2] = KM[k2, k1] = 0
                  next
                } 
                KM[k1,k2] <- ks$kernel(ks, sts[[k1]], sts[[k2]], ksize)
                KM[k2,k1] <- Conj(KM[k1,k2])
            }
        }
        return(KM)
    } else {
        N2 <- length(sts2)
        KM <- matrix(0, nrow=N, ncol=N2) 
        for(k1 in 1:N) {
            for(k2 in 1:N2) {
                KM[k1,k2] = ks$kernel(ks, sts[[k1]], sts2[[k2]], ksize)
            }
        }
        return(KM)
    }
}

kernelFactory <- function(kern_str, T, param1, param2 = NULL) {
    ks <- list(T = T)
    if(kern_str == "binned_lin") {
        ks$kernel = binnedLinear       
    } else 
    if(kern_str == "mci") {
        ks$kernel = mci
    } else 
    if(kern_str == "nci") {
        
    }
    return(ks)
}

