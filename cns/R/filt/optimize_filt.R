
#source('filt_funcs.R')
#source('test_filt.R')

L = 100
y = matrix(sp, nrow=L)
x = matrix(x_ts, nrow=L)


w = as.matrix(exp(-(1:L)/10), nrow=L) # default filter

cut_window = function(i,x) {
    w_i = i:(i+L-1)
    w_i = w_i[w_i>0]
    
    return(matrix(x[w_i], nrow=length(w_i)))
}

E = Vectorize(function(i, x, y, w) {
    xc = cut_window(i,x)
    yc = cut_window(i,y)
    wc = matrix(w[1:length(xc)], nrow=length(xc))
    
    (xc- wc %*% yc)^2
},"i")

dEdw = function(i, x, y, w) {
    w_i = i:(i+L-1)
    w_i = w_i[w_i>0]
    I = rep(1, L)
    gr = -2 * t(y[w_i]) %*% (x[w_i] - y[w_i] %*% t(w[1:length(w_i)]))
    c(-gr)
}


alpha = 0.00001

plot(E(i,x,y,w), type="l")

i=1
for(ep in 1:100) {
    w = w - alpha * dEdw(i,x,y,w)    
    cat("Error: ", sum(E(i,x,y,w)), "\n")
}


