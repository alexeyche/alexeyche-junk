#!/usr/bin/RScript

data <- read.csv("ex2data2.txt", sep=",", header=F)

x.d <- data[,1:2]
x0 <- subset(data, data[,3] == 0)
x1 <- subset(data, data[,3] == 1)
y.d <- data[,3]

m <- length(y.d)

y0 <- subset(y.d, y.d == 0)
y1 <- subset(y.d, y.d == 1)
Py0 <- length(y0) / m
Py1 <- length(y1) / m


distance <- function(x,x.c) {
    w <- 1
    p <- 2
    sum( w * abs( f(x) - f(x.c) )^p )^1/p
}

p <- function(x.i, x.all, h) {
    val <- abs(x.i - x.all)/h
    sum(1/2 * subset( val, val < 1 )) / (m*h)
}

test_p <- function(p, x.all) {
    x.test <- seq(-1,1, by=0.05)
    y.test <- NULL
    for(i in 1:length(x.test)) {
        y.test <- rbind(y.test, p(x.test[i], x.all, 1))
    }
    plot(x.test, y.test)
}    
#x.test <- 
#p(
#x.test <- c(0,0)
#
#p0t.x1 <- p(x.test[1], x0[,1], 0.5)  # P(x1 | y == 0)
#p1t.x1 <- p(x.test[1], x1[,1], 0.5)  # P(x1 | y == 1)
#
#p0t.x2 <- p(x.test[2], x0[,2], 0.5)  # P(x2 | y == 0)
#p1t.x2 <- p(x.test[2], x1[,2], 0.5)  # P(x2 | y == 1)
#
#x1_is_0 <- p0t.x1 * Py0 
#x1_is_1 <- p1t.x1 * Py1
#x2_is_0 <- p0t.x2 * Py0
#x2_is_1 <- p1t.x2 * Py1
#
#cat(x1_is_0, x1_is_1, x2_is_0, x2_is_1, '\n')
#
#if(x1_is_0 > x1_is_1 & x2_is_0 > x2_is_1) {
#    print('x.test is 0 class')
#}
#if(x1_is_0 < x1_is_1 & x2_is_0 < x2_is_1) {
#    print('x.test is 1 class')
#}

plot(x0[,1],x0[,2], col = "green")
points(x1[,1],x1[,2], col = "red")
points(x.test[1],x.test[2])
legend(0.7,1,c("0 class","1 class"), lwd=c(1,1),col=c("green","red")) # gives the legend lines the correct color and width
