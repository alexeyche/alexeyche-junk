#!/usr/bin/RScript

mapFeature <- function(x, degree) {
    if (degree > 1) { 
        for(i in 2:degree) {
            x <- cbind(x, x[,1]^i)
        }
    }
    x <- cbind(rep(1,nrow(x)), x)
    return(x)
}

featureNormalize <- function(x) {
    means <- apply(x,2,mean)
    sds <- apply(x,2,sd)
    x <- t((t(x) - means )/ sds)
    return(x)
}

cookFeature <- function(x, degree) {
    x <- mapFeature(x, degree)
    x <- featureNormalize(x)
}
