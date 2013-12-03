#!/usr/bin/RScript


t_neg <- 34 # ms
t_pos <- 17 # ms

W <- Vectorize(function(u) {
    if(u<0) {
        -exp(u/t_neg)
    } else {
        exp(-u/t_pos)
    }
})

t <- seq(-150,150)

plot(t,W(t), type="l")
