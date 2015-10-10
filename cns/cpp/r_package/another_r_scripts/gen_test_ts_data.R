
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

sample_len = 119

lts = list()

for(i in 1:50) {
    d <- ornstein_uhlenbeck(10,sample_len, 0.2, 1, 0.007, 0.15)
    lts[[length(lts)+1]] = list(ts=d, label="1")
}

for(i in 1:50) {
    d <- ornstein_uhlenbeck(10,sample_len, 0.15, 0.5, 0.01, 0.21)
    lts[[length(lts)+1]] = list(ts=d, label="2")
}



normalize = function(x, min_val, max_val) {
    2*( 1 - (max_val-x)/(max_val-min_val) ) -1
}


require(Rsnn)
sourc
dest_file = "/home/alexeyche/prog/sim/test_data.2_classes.pb"
pr = RProto$new(dest_file) 

max_d = max(sapply(lts,function(x) max(x$ts)))
min_d = min(sapply(lts,function(x) min(x$ts)))
for(i in 1:length(lts)) {    
    lts[[i]]$ts = normalize(lts[[i]]$ts, min_d, max_d)
}

pr$write("LabeledTimeSeriesList", lts)

