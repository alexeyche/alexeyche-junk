#!/usr/bin/RScript

set.seed(0)
library(R.matlab)

num_digits <- 10
batchsize <- 100
if( ! file.exists(".RData")) {
    digitdata <- NULL
    targets <- NULL

    empty_target <- rep(0, num_digits)
    
    for(i in 0:9) {
        filename <- sprintf("digit%d.mat",i)
        d <- readMat(filename)
        m <- nrow(d$D)
        digitdata <- rbind(digitdata, d$D)
        target <- empty_target
        target[i+1] <- 1   # 1 0 0 0 0 0 0 0 0 0 for first case
        targets <- rbind(targets, matrix(rep(target,m), ncol=num_digits, byrow=TRUE))
    }
    digitdata <- digitdata/255  # normalize
    
    totnum <- nrow(digitdata)
    printf('Size of the training dataset = %5d \n', totnum);
    randomorder <- sample(totnum)

    numbatches <- ceiling(totnum/batchsize)
    numdims <- ncol(digitdata)
    batchdata <- array(0, dim=c(batchsize, numdims, numbatches))
    batchtargets <- array(0, dim=c(batchsize,num_digits, numbatches))

    for(b in 1:numbatches) {
        batchdata[,,b] <- digitdata[ randomorder[(1+(b-1)*batchsize):(b*batchsize)], ]
        batchtargets[,,b] <- targets[ randomorder[(1+(b-1)*batchsize):(b*batchsize)], ]
    }
    save.image()
} else {
    load(".RData")
}



