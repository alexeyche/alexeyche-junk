
#source('filt_funcs.R')
#source('test_filt.R')

L = 50
y = matrix(sp, nrow=length(sp))
x = matrix(x_ts, nrow=length(x_ts))



w = as.matrix(exp(-(1:L)/2), nrow=L) # default filter
#w = rep(0, L)

cut_window = function(i,x) {
    w_i = rev((i-L+1):i)
    w_i = w_i[w_i>0]
    
    return(matrix(x[w_i], nrow=length(w_i)))
}

conv = Vectorize(function(i, y, w) {
    yc = cut_window(i,y)
    wc = matrix(w[1:length(yc)], nrow=length(yc))
    
    t(wc) %*% yc
}, "i")

E = Vectorize(function(i, x, y, w) {
    (x[i]- conv(i,y,w))^2
},"i")

dEdw = Vectorize(function(i, x, y, w) { # x = d
    yc = cut_window(i,y)
    
    -2 * ( x[i] - conv(i, y, w) ) * yc
},"i")

mean_area = (L):(L+400)

m_E = function(w) {
    mean(  E(mean_area,x,y,w) ) 
}
m_dEdw = function(w) {
    dEdw_whole = dEdw(mean_area, x, y, w)
    rowMeans(dEdw_whole)
}

simple_gd = function(max_ep, alpha) {
    for(ep in 1:max_ep) {
        w = w - alpha * m_dEdw(w)
        cat("Error: ", m_E(w), "\n")
    }
    return(w)
}    


#opt_res = optim(w, m_E, m_dEdw, method="CG",control=list(trace=5), hessian=FALSE)
#w_opt = opt_res$par



plot(conv(mean_area,y,w_opt), type="l")

