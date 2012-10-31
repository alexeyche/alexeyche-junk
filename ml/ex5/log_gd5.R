#!/usr/bin/RScript

source('load_data.R')

source('learn_curves.R')

degree <- 3
x.w <- cookFeature(xdata, degree)
y.w <- y.n
theta <- grad.descent(x.w, y.w, 5000, 0.04, 0)

x.plot <- as.matrix(seq(-3,3, length.out=300))
x.plot <- mapFeature(x.plot, degree)
y.plot <- h(x.plot, theta)


plot(x.plot[,2], y.plot, type="l", col="red", ylim=c(0,40))
