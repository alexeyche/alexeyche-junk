
ornstein_uhlenbeck <- function(T, n, nu,lambda,sigma,x0){
  dt  <- T/n
  dw  <- rnorm(n, 0, sqrt(dt))
  x <- c(x0)
  for (i in 2:(n+1)) {
    x[i]  <-  x[i-1] + lambda*(nu-x[i-1])*dt + sigma*dw[i-1]
  }
  return(x);
}
path <- "/home/alexeyche/prog/sim/ts"
#path <- "/home/alexeyche/my/sim"
gen_ou <- function() {
    set.seed(8)
    d1 <- ornstein_uhlenbeck(10,999, 0.2, 1, 0.007, 0.15)
    d2 <- ornstein_uhlenbeck(10,999, 0.19, 0.5, 0.01, 0.15)
    d3 <- ornstein_uhlenbeck(10,999, 0.15, 0.5, 0.01, 0.21)
    write.table(d1, sprintf("%s/ou_test/d1.csv",path), sep=",", col.names=F, row.names=F) 
    write.table(d2, sprintf("%s/ou_test/d2.csv",path), sep=",", col.names=F, row.names=F)
    write.table(d3, sprintf("%s/ou_test/d3.csv",path), sep=",", col.names=F, row.names=F)  
    for(i in 1:10) {
        set.seed(i+10)
        d1c <- ornstein_uhlenbeck(10,999, 0.2, 1, 0.007, 0.15)
        d2c <- ornstein_uhlenbeck(10,999, 0.19, 0.5, 0.01, 0.15)
        d3c <- ornstein_uhlenbeck(10,999, 0.15, 0.5, 0.01, 0.21)
        write.table(d1c, paste(path,"/ou_test/d1_",i,".csv",sep=""), sep=",", col.names=F, row.names=F) 
        write.table(d2c, paste(path,"/ou_test/d2_",i,".csv",sep=""), sep=",", col.names=F, row.names=F)
        write.table(d3c, paste(path,"/ou_test/d3_",i,".csv",sep=""), sep=",", col.names=F, row.names=F) 
    }
    
    plot(d1, type="l", col="blue")
    lines(d2, col="red")
    lines(d3, col="green")
}
gen_sd <- function() {
    sd1 <- seq(1, 50)
    sd2 <- seq(50, 1)
    write.table(sd1, paste(path,"/stimuli/sd1.csv",sep=""), sep=",", col.names=F, row.names=F)
    write.table(sd2, paste(path,"/stimuli/sd2.csv",sep=""), sep=",", col.names=F, row.names=F)
}    


#source("serv.R")
#load()

plot_syn_stat <- function(stat, new_plot=TRUE, clr="blue") {
  pf <- plot
  if(!new_plot) { pf <- points }
  for(i in 1:nrow(stat)) {
    cl <- stat[i,stat[i,]>0]
    pf(cl, rep(c(i),length(cl)), col=clr, xlim=c(0, max(stat)), ylim=c(1,nrow(stat)))
    if(i == 1) { pf <- points }    
  }  
}
plot_syn_stat_list <- function(stat_list, new_plot=TRUE, clr="blue") {
    pf <- plot
    if(!new_plot) { pf <- points }
    for(i in 1:length(stat_list)) {
        cl <- stat_list[[i]]
        pf(cl, rep(c(i),length(cl)), col=clr, xlim=c(0, max(unlist(stat_list))), ylim=c(1,length(stat_list)))
        if(i == 1) { pf <- points }    
    }  
}
#par(mfrow=c(1,2))
#plot_syn_stat(d1_stat,TRUE,clr="blue")
#plot_syn_stat(d2_stat,TRUE,clr="red")
#require(stringr)
#l <- list()
#for(i in ls()) {
#    m <- str_match(i, "d_stat([0-9]+)")
#    if(!is.na(m[2])) {
#        l[[m[1]]] <- get(m[1])        
#    }
#}





