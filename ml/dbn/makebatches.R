#!/usr/bin/RScript


load_data_and_make_bacthes <- function() {
    set.seed(0)
    library(R.matlab)
    
    num_digits <- 10
    batchsize <- 100
 #   if( ! file.exists(".RData")) {
        digitdata <- NULL
        digitdata.t <- NULL
        targets <- NULL
        targets.t <- NULL
        
        empty_target <- rep(0, num_digits)
        
        for(i in 0:9) {
            filename <- sprintf("digit%d.mat",i)
            filename.t <- sprintf("test%d.mat",i) # .t means test data
            
            d <- readMat(filename)
            d.t <- readMat(filename.t)
            
            m <- nrow(d$D)
            m.t <- nrow(d.t$D)
            
            digitdata <- rbind(digitdata, d$D)
            digitdata.t <- rbind(digitdata.t, d.t$D)
            
            target <- empty_target
            target.t <- empty_target
            
            target[i+1] <- 1   # 1 0 0 0 0 0 0 0 0 0 for first case
            target.t[i+1] <- 1   # 1 0 0 0 0 0 0 0 0 0 for first case
            # target and target.t are the same bec.of way to read files, but we specify theirs separatly for the sake of clear
            
            targets <- rbind(targets, matrix(rep(target,m), ncol=num_digits, byrow=TRUE))
            targets.t <- rbind(targets.t, matrix(rep(target.t,m.t), ncol=num_digits, byrow=TRUE))
        }
        digitdata <- digitdata/255  # normalize
        digitdata.t <- digitdata.t/255  # normalize
        
        totnum <- nrow(digitdata)
        totnum.t <- nrow(digitdata.t)
        printf('Size of the training dataset = %5d: ', totnum, ', and of the test dataset: ', totnum.t, '\n')
        randomorder <- sample(totnum)
        randomorder.t <- sample(totnum.t)
        
        numbatches <- ceiling(totnum/batchsize)
        numbatches.t <- ceiling(totnum.t/batchsize)
        
        numdims <- ncol(digitdata)
        numdims.t <- ncol(digitdata.t)
        batchdata <- array(0, dim=c(batchsize, numdims, numbatches))
        batchtargets <- array(0, dim=c(batchsize, num_digits, numbatches))
        testbatchdata <- array(0, dim=c(batchsize, numdims.t, numbatches.t))
        testbatchtargets <- array(0, dim=c(batchsize, num_digits, numbatches.t))
        
        for(b in 1:numbatches) {
            batchdata[,,b] <- digitdata[ randomorder[(1+(b-1)*batchsize):(b*batchsize)], ]
            batchtargets[,,b] <- targets[ randomorder[(1+(b-1)*batchsize):(b*batchsize)], ]
            if(b<=numbatches.t) {
                testbatchdata[,,b] <- digitdata.t[ randomorder.t[(1+(b-1)*batchsize):(b*batchsize)], ]
                testbatchtargets[,,b] <- targets.t[ randomorder.t[(1+(b-1)*batchsize):(b*batchsize)], ]
            }
        }
        save.image()
#    } else {
 #       load(".RData")
#    }
     list(batchdata = batchdata, batchtargets = batchtargets, testbatchdata = testbatchdata, testbatchtargets = testbatchtargets)
}


