#!/usr/bin/RScript

x <- cbind( sample( c(rnorm(100, mean=5, sd=3), rnorm(100, mean=-5,sd=3)) ),
            sample( c(rnorm(130, mean=2, sd=0.5), rnorm(70, mean=0,sd=1)) )
          )  


m <- nrow(x)
n <- ncol(x)

N <- function(x, mu, E) {
    n <- ncol(x)
    (2*pi)^(-n/2) * det(E)^(-1/2) * exp(-(1/2) * (x-mu) %*% solve(E) %*% t(x-mu) )
}

"%-%" <- function(x,y) {
    sweep(x,2,y, FUN="-")
}   
# g - weight for each elemnt in x
# w - weight for current density
mean.reg <- function(x) {
    apply(x, 2, sum)/nrow(x)
}

mean.w <- function(x,g,w) {
    mean.mix <- array(dim=c(1,ncol(x),ncol(g)))
    for(j in 1:ncol(g)) {
        mean.mix[,,j] <- apply(x * g[,j], 2, sum) / (nrow(x)*w[j])
    }
    return(mean.mix)
}

cov.w <- function(x, mean, g, w) {
    m <- nrow(x)
    cov.mix <- array(dim=c(ncol(x),ncol(x),ncol(g)))
    for(j in 1:ncol(g)) {
        s <- 0
        for(i in 1:m) {
            s = s + g[i,j] * (x[i,]-mean[,,j]) %*% t(x[i,]-mean[,,j])
        }
        cov.mix[,,j] <- s/(m*w[j])
    }
    return(cov.mix)
}

k <- 3
g <- matrix(1/k,nrow=m, ncol=k)
w <- matrix(1/k,nrow=1, ncol=k)

mean.whole <- mean.reg(x)
sd.whole <- cbind(sd(x[,1]),sd(x[,2]))

# for each k:
mean.start <- array(dim=c(1,n,k))
cov.start <- array(dim=c(n,n,k))
for(i in 1:n) {
    mean.start[,i,1] <- mean.whole[i] - sd.whole[i]/2
    mean.start[,i,2] <- mean.whole[i]
    mean.start[,i,3] <- mean.whole[i] + sd.whole[i]/2
}

cov.start <- cov.w(x,mean.start,g,w)

calc_g <- function(g, x, mean, cov) {
    m <- nrow(x)
    for(i in 1:m) {
        sum_p = 0 
        x.i <- t(as.matrix(x[i,]))
        den <- matrix(ncol=k)
        for(j in 1:k) {
            den[j] = w[j] * N(x.i,mean[,,j], cov[,,j])
        }
        for(j in 1:k) {
            g[i,j] <- den[j]/sum(den)
        }
    }
    return(g)
}

em <- function(x, mean.start, cov.start, delta.min) {
    mean.calc <- mean.start
    cov.calc <- cov.start
    while(T) {
        g0 <- g
    # Expectation
        g <- calc_g(g, x, mean.calc , cov.calc)
        w <- apply(g,2,sum)/m
    # Maximization
        mean.calc <- mean.w(x,g,w)
        cov.calc <- cov.w(x,mean.calc,g,w)
        delta <- max(g0-g)
#        print(delta)
        if (delta<=delta.min) {
            break
        }
    }
    list(mean = mean.calc, cov = cov.calc)
}


plot_gauss <- function(mean,cov) {
    x1n <- 200
    x2n <- 200
    x1.plot <- seq(-2,2,length.out=x1n)
    x2.plot <- seq(-2,2,length.out=x2n)
    z <- matrix(nrow=x1n,ncol=x2n)
    for(i in 1:x1n) {
        for(j in 1:x2n) {
            z[i,j] <- N(cbind(x1.plot[i],x2.plot[j]), mean, cov)
        }
    }    
    contour(x1.plot,x2.plot,z=z)
}

ret <- em(x,mean.start,cov.start,0.01)
par(mfrow=c(1,4))
for(j in 1:k) {
    plot_gauss(ret$mean[,,j],ret$cov[,,j])
}
plot(x[,1],x[,2])
