#!/usr/bin/RScript

featureNormalize <- function(x) {
    means <- apply(x,2,mean)
    sds <- apply(x,2,sd)
    x <- t((t(x) - means )/ sds)
    return(x)
}

makebatches <- function( data, target.data, batch.size = ceiling(nrow(data)/10), normalize = TRUE, permutate = TRUE ) { 
    stopifnot(nrow(data) == nrow(target.data))
    
    if(normalize) {
        data <- featureNormalize(data)
    }   
    
    num.dims <- ncol(data)
    num.dims.target <- ncol(target.data)
    tot.num <- nrow(data)
    num.batches <- ceiling(tot.num/batch.size)
    if(permutate) { 
      randomorder <- sample(tot.num)
    }  
    
    batch.data <- array(0, dim=c(batch.size, num.dims, num.batches))
    batch.targets <- array(0, dim=c(batch.size, num.dims.target, num.batches))
    
    for(b in 1:num.batches) {
        if(permutate) {
          batch.data[,,b] <- data[ randomorder[(1+(b-1)*batch.size):(b*batch.size)], ]
          batch.targets[,,b] <- target.data[ randomorder[(1+(b-1)*batch.size):(b*batch.size)], ]
        } else {
          batch.data[,,b] <- data[ (1+(b-1)*batch.size):(b*batch.size), ]
          batch.targets[,,b] <- target.data[ (1+(b-1)*batch.size):(b*batch.size), ]
        }
    }   
    return(list(batch.data,batch.targets))
}

