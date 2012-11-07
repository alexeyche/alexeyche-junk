#!/usr/bin/RScript

exp_d <- function(x, b) {
    1 - exp(-x/b)
}

plot_exp_d <- function() {
    par(mfrow = c(3,3))
    for(b in seq(0.1,1.7,by=0.2)) {
        x.plot <- seq(-1,1,by=0.05)
        y.plot <- exp_d(x.plot, b)
        plot(x.plot, y.plot)
    }
}


gamma_d_p <- function(x, alpha, beta) {
    (1/(factorial(alpha) * beta^(alpha+1))) * x^alpha * exp(-x/beta)     
}

gamma_d <- function(x, alpha, beta) {
    alpha_v <- 0:alpha
    out <- NULL
    for(i in 1:length(x)) {
        out <- rbind(out, 1 - sum( exp(-x[i]/beta) * (x[i]/beta)^alpha_v * 1/factorial(alpha_v)))
    }
    return(out)
}

plot_gamma_d <- function() {
    par(mfrow = c(2,1))
    b <- 10^5
    a <- 3
    x.plot <- seq(0,400000, by=10000)
    y.plot <- gamma_d(x.plot,a,b)
    plot(x.plot, y.plot)
    x.plot.p <- seq(0,400000, by=10000)
    y.plot.p <- gamma_d_p(x.plot.p, a, b)
    plot(x.plot.p, y.plot.p)
}

plot_gamma_d()
