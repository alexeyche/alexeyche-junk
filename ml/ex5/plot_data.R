#!/usr/bin/RScript

plot_hypothesis <- function(theta, degree, x, y) {
    x.plot <- as.matrix(seq(-2, 2, length.out = 300))
    x.plot <- mapFeature(x.plot, degree)
    y.plot <- h(x.plot, theta)
    plot(x, y)
    lines(x.plot[,2], y.plot)
}

plot_learn_curves <- function(st) {
    p <- par(mfrow = c(4,4))
    for( i in 1:length(st) ) {
        data <- st[[i]]
        plot(data[,1],data[,3], ylim = c(0,1), type ="l", col = "blue" )
        lines(data[,1],data[,2], col = "green")
    }
}

