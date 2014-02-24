#!/usr/bin/env RScript

# getN = function(x,tt=1) {
#   if(sum(x)<tt)
#     return(NA)
#   if(x[1]>tt)
#     return(0)
#   else
#     return(max(which(cumsum(x)<tt)))
# }
# 
# 
# #x = c(0.1,0.6,0.5,0.3,1.4)
# #getN(x)
# simPP = function(ntrials,tt=1,lambda=2) {
#   maxN = round(qpois(0.99999,tt*lambda))+10 # large enough so that the chance of needing a larger sample is tiny
#   x = matrix(rexp(ntrials*maxN,lambda),ntrials,maxN)
#   return(apply(x,1,getN,tt=tt))
# }
require(sde)
draw_pp <- function(ntrials, lambda, t_max) {
  k <- 0; t<-0
  k <- numeric(ntrials)
  while(TRUE){
    u=runif(ntrials)
    t=t-log(u)/lambda
    k_cont <- t <= t_max
    if(all(k_cont == FALSE)) { 
      break
    }
    k[k_cont] <- k[k_cont] + 1
  }
  return(k)
}



ornstein_uhlenbeck <- function(T, n, nu,lambda,sigma,x0){
  dt  <- T/n
  dw  <- rnorm(n, 0, sqrt(dt))
  x <- c(x0)
  for (i in 2:(n+1)) {
    x[i]  <-  x[i-1] + lambda*(nu-x[i-1])*dt + sigma*dw[i-1]
  }
  return(x);
}


require(sde)


rect <- Vectorize(function(x) if((x) < 0)  0  else x )

build_corr_matrix <- function(coeff, r) {
  C_ast <- matrix(coeff, ncol=2, nrow=2)
  diag(C_ast) <- 0
  D <- diag(x = r^2, length(r))
  alpha <- -min(eigen(solve(D) %*% C_ast)$value)
  C <- C_ast
  diag(C) <- alpha*r^2 + 0.01
  return(C)
}

generate_corr_spikes <- function(T, dt, r, C) {
  L <- t(chol(C))
  y1 <- rcOU(T/dt-1, dt, 0, c(r[1],1,1))
  y2 <- rcOU(T/dt-1, dt, 0, c(r[2],1,1))
  y <- cbind(y1,y2)
  x <- matrix(0, ncol=2, nrow=1)
  spikes <- NULL
  for(i in (2:(T/dt))) {
    x <- rbind(x, rect(r + L %*%  y[i-1,]))  
    coin <- runif(2)
    spikes <- rbind(spikes, as.integer(coin <= x[i]*dt))
  }
  return(spikes)
}



ccf <- function(spikes, dt, s) {
  i <- 1; j <- 2
  if(s<0) {
    s <- abs(s)
    i<-2; j<-1
  }
  s_i <- s/dt
  
  mean(spikes[1:(9999-s_i),i]*spikes[(s_i+1):9999,j])
}

ccvf <- function(spikes, dt, s) {
  i <- 1; j <- 2
  if(s<0) {
    s <- abs(s)
    i<-2; j<-1
  }
  s_i <- s/dt
  mean(spikes[1:(length(spikes[,i])-s_i),i]*spikes[(s_i+1):length(spikes[,j]),j]) - mean(spikes[,1])*mean(spikes[,1])
}

test <- function(r = c(0.05,0.05), coeff = 1) {
  C <- build_corr_matrix(1, r)
  S <- generate_corr_spikes(10000, coeff, r, C)
  
  ss <- seq(-100, 100, by=1)
  ccf_out<-NULL; for(sss in ss) { ccf_out <- c(ccf_out, ccf(S, 1, sss)) }
  plot(ss, ccf_out, type="l")
}


correl_poiss = function (lambda, corr, n=10000)
{
  
  merke = c()
  aussortieren = c()
  
  for (i in lambda)
  {
    merke=cbind(merke,runif(n+length(lambda)))
  }
  
  merke=merke%*%base::chol(corr)
  
  for (i in 1:length(lambda))
  {
    merke[,i]=merke[,i] / sort(merke[,i],decreasing=TRUE)[1]
    merke[,i]=qpois(merke[,i],lambda[i])
    aussortieren = c(aussortieren,which(merke[,i]==max(merke[,i])))
  }
  return(merke[-aussortieren,])
}

lx = rpois(10000, 8)
ly = rpois(10000, 8)
lt = rpois(10000, 2)

x = lx+lt
y = ly+lt
