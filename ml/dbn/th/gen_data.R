#!/usr/bin/RScript

two_binom <- function(file = "two_binom.csv", num.cases = 1000, num.dims = 50) {
  data.all <- NULL
  for(c in 1:num.cases) {
    m <- matrix(0, ncol = num.dims)
    rb <- rbinom(num.dims,num.dims,0.9)
    rb2 <- rbinom(num.dims,num.dims,0.1)
    for(i in 1:length(rb)) {
      m[rb[i]] = m[rb[i]]+1
      m[rb2[i]] = m[rb2[i]]+1          
    }
    m <- m/max(m)
    data.all <- rbind(data.all, m)
  }
  write.table(data.all, file, sep=",", row.names=FALSE, col.names=FALSE)  
  cat(file)
}

args <- commandArgs(trailingOnly = TRUE)
if (length(args) != 0) {
    fun <- args[1]
    if (fun == "two_binom") {
        two_binom(num.cases = as.integer(args[2]), num.dims = as.integer(args[3]))
    }
}


