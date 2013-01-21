#!/usr/bin/RScript

# example
get_var <- function(mu) {
  if(runif(1)<0.9) {
    rnorm(1,mean = mu)  
  } else {
    runif(1,min=-5+mu, 5+mu)
  }
}

n = 15
mu1 = 0
mu2 = -2
n_sample <- 1000

p.v <- c()
for(i in 1:n_sample) {
    x <- y <- c()
    for(i in 1:15) {
        x <- c(x, get_var(mu1))
        y <- c(y, get_var(mu2))
    }    
    p.v <- c(p.v, t.test(x,y)$p.value)
}    
for(i in 1:15) {
        x <- c(x, get_var(mu1))
        y <- c(y, get_var(mu2))
}


cs <- cumsum(p.v)/n_sample
