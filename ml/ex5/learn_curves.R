#!/usr/bin/RScript

source('load_data.R')
source('grad_desc.R')

get_stats <- function(x, y, x_cv, y_cv, it, alpha) {
    stats <- NULL
    m <- nrow(x)
    n <- ncol(x)
    for(i in 2:m) {
        x_temp <- x[1:i,]
        y_temp <- y[1:i,]
        theta <- grad.descent(x_temp, y_temp, it, alpha, 0)
        
        stats <- rbind( stats, c( i, err(x_temp, y_temp, theta), err(x_cv, y_cv, theta) ) )
    }
    return(stats)
}

learn_curves <- function(maxpow = 15) {
    st <- NULL
    for( i in 1:maxpow ) {
        x.w <- cookFeature(x.n, i)
        y.w <- y.n
        x.cv <- cookFeature(xval.n, i)
        y.cv <- yval.n
        n <- ncol(x.w)    
        st[[i]] <- get_stats(x.w, y.w, x.cv, y.cv, 5000, 0.04)
    }
    return(st)
}

#st <- learn_curves()
#save.image()
#load('.RData')

#source('plot_data.R')
#plot_learn_curves(st)
