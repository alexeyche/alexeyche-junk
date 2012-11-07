#!/usr/bin/RScript

data(iris)
test <- iris[c(48:50,98:100,148:150),]
iris <- iris[c(1:47,51:97,101:147),]
x.d <- iris[,1:4]
y.d <- iris[,5]
y <- levels(iris[,5])
y.l <- length(y)

x.test <- test[1:4]

n <- ncol(x.d)
m <- nrow(x.d)

source("../featureCook.R")

x.d.n <- featureNormalize(x.d)
test.n <- featureNormalize(test[,1:4])

x.d <- as.matrix(x.d)

mean_weight <- function(x, g)  {
   matrix(apply(g*x,2,sum)/sum(g), nrow=1)
}

x.y <- G.y <- cov <- mw <- NULL
for(i in 1:y.l) {
    x.y[[i]] <- subset(x.d, y.d == y[i])
    G.y[[i]] <- rep(1, nrow(x.y[[i]]) )  # weight picked up by 1
    mw[[i]] <- mean_weight(x.y[[i]], G.y[[i]])
}
names(x.y) <- names(G.y) <- names(mw) <- c('c1','c2','c2')

gen_cov_matrix <- function(x, m, g) {
   sweep(x.y$c1, 2, mw$c1, "-") 
}



