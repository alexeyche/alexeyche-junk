#!/usr/bin/RScript


data(iris)
x.d <- iris[,1:4]
y.d <- iris[,5]
y <- levels(iris[,5])

n <- ncol(x.d)
m <- nrow(x.d)

# vector form
K <- function(r) {
  ignore <- abs(r) > 1
  r <- 3/4 * (1-r^2)
  r[ignore] <- 0
  return(r)
}

# not a vector form, Euclid in default
#dist_mink <- function(x, x.all, w = 1, p = 2) {
#    sum( w * abs(x.all - x)^p ) ^ (1/p)
#}

parzen_window <- function(x, x.all, h, m) {
  kernels_pr <- 1
  for(j in 1:n) {
    kernels_pr <- kernels_pr * K(abs(x.all[,j] - x[,j])/h)/h
  }
  sum(kernels_pr)/m
}
naive_bayes <- function(x,Xl,h) {

for(spec in y) {
  x.w <- subset(x.d,y.d == spec)
  #normalization
  for(i in 1:n) {
    x.w[,i] <- (x.w[,i] - mean(x.w[,i]))/sd(x.w[,i])
  }
  p.w.c <- NULL
  for(i in 1:m) {
    h <- 0.3
    p.w.c <- rbind(p.w.c, parzen_window(x.w[i,],x.w,h,m))
  }
  p.w <- cbind(p.w,p.w.c)
}
#for(i in 1:n) {
#  hist(x.w[,i])
#}


test_dist_mink <- function() {
  print(dist_mink(c(5),c(1:10)))  
}

test_parzen_window <- function() {
  x <- 5
  x.all <- c(1:10)
  print(parzen_window(x,x.all, K, dist_mink,2,length(x.all)))
}
plot_K <- function() {
  x.plot <- seq(-2,2,by=0.05)
  #y.plot <- NULL
  #for(i in x.plot) {
  #  y.plot <- rbind(y.plot, K(i))
  #}
  y.plot <- K(x.plot)
  plot(x.plot, y.plot)
}
