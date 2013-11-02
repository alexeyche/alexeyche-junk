

ornstein_uhlenbeck <- function(T, n, nu,lambda,sigma,x0){
  dt  <- T/n
  dw  <- rnorm(n, 0, sqrt(dt))
  x <- c(x0)
  for (i in 2:(n+1)) {
    x[i]  <-  x[i-1] + lambda*(nu-x[i-1])*dt + sigma*dw[i-1]
  }
  return(x);
}

set.seed(6)
d1 <- ornstein_uhlenbeck(10,1000, 0.2, 1, 0.007, 0.15)
d2 <- ornstein_uhlenbeck(10,1000, 0.19, 0.5, 0.01, 0.15)
write.table(d1, "/var/tmp/d1.csv", sep=",", col.names=F, row.names=F) 
write.table(d2, "/var/tmp/d2.csv", sep=",", col.names=F, row.names=F)
  
#plot(d1, type="l", col="blue")
#lines(d2, col="red")

#source("serv.R")
load()

plot_syn_stat <- function(stat, new_plot=TRUE, clr="blue") {
  pf <- plot
  if(!new_plot) { pf <- points }
  for(i in 1:nrow(stat)) {
    cl <- stat[i,stat[i,]>0]
    pf(cl, rep(c(i),length(cl)), col=clr, xlim=c(1, nrow(stat)), ylim=c(0,max(stat)))
    if(i == 1) { pf <- points }    
  }  
}
par(mfrow=c(1,2))
plot_syn_stat(d1_stat,TRUE,clr="blue")
plot_syn_stat(d2_stat,TRUE,clr="red")





