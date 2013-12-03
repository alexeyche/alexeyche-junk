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
        x.w <- mapFeature(x.n, i)
        y.w <- y.n
        x.cv <- mapFeature(xval.n, i)
        y.cv <- yval.n
        
        st[[i]] <- get_stats(x.w, y.w, x.cv, y.cv, 2000, 0.01)
    }
    return(st)
}

select_lambda <- function(maxlambda = 10) {
    lambda <- 0.5
    stats <- NULL
    for(i in seq(0.5,2, by=0.1)) {
        degree <- 5
        x.m <- mapFeature(x.n, degree)
        y.m <- y.n
        x.cv <- mapFeature(xval.n, degree)
        y.cv <- yval.n
        
        theta <- grad.descent(x.m, y.m, 2000, 0.05, i)
        
        stats <- rbind( stats, c( i, err(x.m, y.m, theta), err(x.cv, y.cv, theta) ) )
    }
    return(stats)
}

#st <- learn_curves()
st <- select_lambda()
#save.image()
#load('.RData')

#source('plot_data.R')
#plot_learn_curves(st)
