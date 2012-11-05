#x <- seq(-200,400, by=1)
#y <- dnorm(x, mean=100, sd=50)
#plot(x,y)

norm_d_func <- function(z) {
  a <- c(0.31938153, -0.35656378, 1.7814779, -1.821256, 1.3302744)
  b <- c(0.2316419)
  degrees <- 1:5
  lambda <- (1 + b*z)^-1
  1 - (sqrt(2*pi)*exp((z^2)/2))^-1 * sum(a*lambda^degrees)
}

plot_d <- function(f) {
  x.plot <- seq(-1.457,2, by=0.1)
  y.plot <- NULL
  for(i in x.plot) { 
    y.plot<-rbind(y.plot, f(i))
  }
  plot(x.plot, y.plot)
}

unif_d_density <- function(x,a,b) {
  if((x>b)&(x<a)) return(0)
  return((b-a)^-1)
}

plot_d_unif <- function() {
  x.plot <- seq(-1.457,2, by=0.1)
  y.plot <- NULL
  for(i in x.plot) { 
    y.plot<-rbind(y.plot, unif_d_density(i,-2,2))
  }
  plot(x.plot, y.plot)
}

weibull_distr <- function(x,alpha,beta) {
  1- exp(-(x/alpha)^beta)
}
plot_weibull <- function() {
  alpha <- 1
  beta <- 2
  x.plot <- seq(0,30, by=0.1)
  y.plot <- weibull_distr(x.plot,alpha,beta)
  plot(x.plot, y.plot)
}
plot_gamma <- function() {
  x.plot <- seq(0,10, by=0.1)
  y.plot <- NULL
  for(i in x.plot) { 
    y.plot<-rbind(y.plot, dgamma(i,2, 10))
  }
  plot(x.plot, y.plot)
}

comb_with_rep <- function(n,k) {
  factorial(n+k-1)/(factorial(k)*factorial(n-1))
}
comb_with_rep_set <- function(set, k) {
  expand.grid(rep(list(set), k)) 
}

binom_distr <- function(x,n,p) {
  comb_with_rep(n,x) * p^x * (1-p)^(n-x)
}
