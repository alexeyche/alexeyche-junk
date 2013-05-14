#!/usr/bin/RScript

source('dbn_util.R')

num.vis <- 3
num.hid <- 3
num.layers <- 3


hid_states <- array(0, dim=c(num.layers,num.hid))
W.0 <- array(0, dim=c(num.vis,num.hid))
W.1 <- array(0, dim=c(num.hid,num.hid))
W.2 <- array(0, dim=c(num.hid,num.hid))

init <- 0.2

for(i in 1:num.vis) {
    for(j in 1:num.hid) {
        if (abs(i-j) == 1) {            
            W.0[i,j] <- init
            W.1[i,j] <- init
            W.2[i,j] <- init
        }
        if (abs(i-j) == 0) {            
            W.0[i,j] <- init
            W.1[i,j] <- init
            W.2[i,j] <- init
        }
    }
}

v <- matrix(c(0,0.5,0.5), nrow=1, ncol=num.vis)

tr <- function(x) {
    x[x < 0.1] <- 0
    return(x)
}

h.0 <- tr(v) %*% W.0
h.1 <- tr(h.0) %*% W.1
h.2 <- tr(h.1) %*% W.2

hid_states <- rbind(v, h.0,h.1,h.2)
gray_plot(t(hid_states))