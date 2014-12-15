
ornstein_uhlenbeck <- function(T, n, nu,lambda,sigma,x0){
  dt  <- T/n
  dw  <- rnorm(n, 0, sqrt(dt))
  x <- c(x0)
  for (i in 2:(n+1)) {
    x[i]  <-  x[i-1] + lambda*(nu-x[i-1])*dt + sigma*dw[i-1]
  }
  return(x);
}

set.seed(8)
len = 249
d1 <- ornstein_uhlenbeck(10,len, 0.2, 1, 0.007, 0.15)
d2 <- ornstein_uhlenbeck(10,len, 0.2, 1, 0.007, 0.15)
d3 <- ornstein_uhlenbeck(10,len, 0.15, 0.5, 0.01, 0.21)
d4 <- ornstein_uhlenbeck(10,len, 0.15, 0.5, 0.01, 0.21)



require(Rsnn)

dest_file = "/home/alexeyche/prog/sim/test_data.4.pb"
pr = RProto$new(dest_file) 

lts = list("list"=list(
    list("ts"=d1, "label"="1"), 
    list("ts"=d2, "label"="1"),
    list("ts"=d3, "label"="2"), 
    list("ts"=d4, "label"="2")
)
)


max_d = max(sapply(lts$list,function(x) max(x$ts)))
min_d = min(sapply(lts$list,function(x) min(x$ts)))
for(i in 1:length(lts$list)) {    
    lts$list[[i]]$ts = 2*(0.21 - lts$list[[i]]$ts)/(max_d-min_d) -1
}

pr$write("LabeledTimeSeriesList", lts)


