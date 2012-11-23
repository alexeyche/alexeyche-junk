#!/usr/bin/Rscript


plot_den <- function(x) {
    n <- ncol(x)
    par(mfrow = c(n,n))
    for(i in 1:n) {
        for(j in 1:n) {
            plot(x[,i],x[,j])
        }
    }
}
