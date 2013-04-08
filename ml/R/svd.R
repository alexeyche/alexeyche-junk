#!/usr/bin/RScript

n <- 100

m <- matrix(sample(10,(n^2+n), replace=TRUE), ncol=n, nrow=n+1)

m.d <- svd(m)