#!/usr/bin/RScript

plot_hypothesis <- function(theta, degree) {
    x <- seq(-40,40, length.out=300)        
    x <- cookFeature(x, degree)
}

plot_learn_curves <- function(st) {
    p <- par(mfrow = c(4,4))
    for( i in 1:length(st) ) {
        data <- st[[i]]
        plot(data[,1],data[,3], ylim = c(0,1), type ="l", col = "blue" )
        lines(data[,1],data[,2], col = "green")
    }
}

