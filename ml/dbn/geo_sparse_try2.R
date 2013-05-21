#!/usr/bin/RScript

source('dbn_util.R')

num.vis <- 10
num.hid <- 8
num.layers <- 3


hid_states <- array(0, dim=c(num.layers,num.hid))
W.0 <- array(0, dim=c(num.vis,num.hid))
W.1 <- array(0, dim=c(num.hid,num.hid))
W.2 <- array(0, dim=c(num.hid,num.hid))
W.3 <- array(0, dim=c(num.hid,num.hid))
W.4 <- array(0, dim=c(num.hid,num.hid))

init <- 0.5 
init_n <- init/2


for(i in 1:num.vis) {
    for(j in 1:num.hid) {
        if (abs(i-j-1) == 1) {            
            W.0[i,j] <- init_n
        }
        if (abs(i-j-1) == 0) {            
            W.0[i,j] <- init
        }
    }
}

for(i in 1:num.hid) {
    for(j in 1:num.hid) {
        if (abs(i-j) == 1) {            
            W.1[i,j] <- init_n
            W.2[i,j] <- init_n 
            W.3[i,j] <- init_n 
            W.4[i,j] <- init_n 
            if((j==1)|(j==num.hid)) {
                W.1[i,j] <- init_n*2
                W.2[i,j] <- init_n*2
                W.3[i,j] <- init_n*2 
                W.4[i,j] <- init_n*2 
            }
        }
        if (abs(i-j) == 0) {            
            W.1[i,j] <- init
            W.2[i,j] <- init 
            W.3[i,j] <- init 
            W.4[i,j] <- init 
        }
    }
}

v <- matrix(c(0,0,0,0,0,0,0,0,0.5,1), nrow=1, ncol=num.vis)
#v <- matrix(1, nrow=1, ncol=num.vis)

tr <- function(x) {
    x[x < 0.1] <- 0
    return(x)
}

to_one <- function(x) {
    x/max(x)
}

prop <- function(v) {
    h.0 <- v %*% W.0
    h.1 <- h.0 %*% W.1
    h.2 <- h.1 %*% W.2
    h.3 <- h.2 %*% W.3
    h.4 <- h.3 %*% W.4
    hid_states <- rbind(h.0,h.1,h.2,h.3,h.4)
    return(hid_states)
}
hid_states <- prop(v)
#gray_plot(t(hid_states))
