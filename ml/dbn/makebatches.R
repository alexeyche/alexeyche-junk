#!/usr/bin/RScript

featureNormalize <- function(x) {
    means <- apply(x,2,mean)
    sds <- apply(x,2,sd)
    x <- t((t(x) - means )/ sds)
    return(x)
}

makebatches <- function( data, target_data, batch.size = ceiling(nrow(data)/10), normalize = TRUE ) { 
    stopifnot(nrow(data) == nrow(target_data))
    
    if(normalize) {
        data <- featureNormalize(data)
    }   
    
    num.dims <- ncol(data)
    num.dims.target <- ncol(target_data)
    tot.num <- nrow(data)
    num.batches <- ceiling(tot.num/batch.size)
    randomorder <- sample(tot.num)
    
    batch.data <- array(0, dim=c(batch.size, num.dims, num.batches))
    batch.targets <- array(0, dim=c(batch.size, num.dims.target, num.batches))
    
    for(b in 1:num.batches) {
        batch.data[,,b] <- data[ randomorder[(1+(b-1)*batch.size):(b*batch.size)] ]
        batch.targets[,,b] <- target_data[ randomorder[(1+(b-1)*batch.size):(b*batch.size)] ]
    }   
    return(list(batch.data,batch.targets))
}

