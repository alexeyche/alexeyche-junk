#source('test_filt.R')

L = 10


y = spikes
x = x_ts
M = nrow(spikes)
#w = matrix(rep(exp(-(1:L)/2),M), nrow=M, ncol=L, byrow=TRUE) # default filter
w = matrix(0, nrow=M, ncol=L) # default filter


cut_window_vec = function(i,x) {
    w_i = rev((i-L+1):i)
    w_i = w_i[w_i>0]
    
    return(matrix(x[,w_i], nrow=M, ncol=length(w_i)))
}

conv_vec = Vectorize(function(i, y, w) {
    yc = cut_window_vec(i,y)
    wc = matrix(w[1:length(yc)], nrow=M, ncol=ncol(yc))
    
    sum(sapply(1:M, function(i) t(wc[i, ]) %*% yc[i,]))
}, "i")

E_vec = Vectorize(function(i, x, y, w) {
    (x[i]- conv_vec(i,y,w))^2
},"i")

dEdw_vec = Vectorize(function(i, x, y, w) { # x = d
    yc = cut_window_vec(i,y)
    
    gr = -2 * ( x[i] - conv_vec(i, y, w) ) * yc
    cbind(gr, matrix(0, M, L-ncol(gr)) )
},"i", SIMPLIFY=FALSE)



mean_area = 1:length(x)

m_E = function(w) {
    mean(  E_vec(mean_area,x,y,w) ) 
}
m_dEdw = function(w) {
    dEdw_whole = dEdw_vec(mean_area, x, y, w)
    Reduce("+", dEdw_whole)/length(dEdw_whole)
}


opt_res = optim(w, m_E, m_dEdw, method="BFGS",control=list(trace=1), hessian=FALSE)
w_opt = opt_res$par

plot(conv_vec(mean_area,y,w_opt), type="l")






