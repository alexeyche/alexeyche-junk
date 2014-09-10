filt = Vectorize(function(s, w) {
    if(s<0) return(0)
    if(s>=L) return(0)
    w[s+1]
},"s")

conv = function(x, w) {
    xv = NULL
    L = length(w)
    for(i in 1:length(x)) {    
        #w_i = (i-L+1):i
        #w_i = w_i[w_i>0]
        w_i = i:(i+L-1)
        w_i = w_i[w_i>0]
        
        xv = c(xv, t(rev(w[1:length(w_i)])) %*% x[w_i])
    }
    return(xv)
}