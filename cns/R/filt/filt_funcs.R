
nextpow2 <- function(x) {
    if (is.null(x) || length(x) == 0) return(c())
    if (!is.numeric(x) && !is.complex(x))
        stop("Argument 'x' must be a numeric/complex vector/matrix.")
    
    x[x == 0] <- 1
    return(ceiling(log2(abs(x))))
}

xcorr = function(x, y=NULL, len) {
    ret_coeff = -1
    if(is.null(y)) {
        y = x
        ret_coeff = 1
    }
    nfft = 2^nextpow2(2*len+1);
    r = fft( fft(x,nfft) * Conj(fft(y,nfft)) , inverse=TRUE);
    r = c(r[(length(r)-len+1):length(r)], r[1:(len+1)]) 
    
    return(ret_coeff*Re(r/length(x)))    
}

toeplitz = function(x,y) {
    if(x[1] != y[1]) stop("Need first elements be equal")
    A = matrix(0, nrow=length(x), ncol=length(y))
    ind = col(A) - row(A)
    xi = which(ind <= 0, arr.ind=TRUE)
    A[xi] <- x[abs(ind[xi])+1]
    yi = which(ind >= 0, arr.ind=TRUE)
    A[yi] <- y[abs(ind[yi])+1]
    return(A)
}



cut_window = function(i,x,L) {
    w_i = rev((i-L+1):i)
    w_i = w_i[w_i>0]
    
    return(matrix(x[w_i], nrow=length(w_i)))
}

conv = Vectorize(function(i, y, w, shift=0) {
    if( (i+shift) <= 0) return(0.0)
    yc = cut_window(i+shift,y, length(w))
    wc = matrix(w[1:length(yc)], nrow=length(yc))
    
    t(wc) %*% yc
}, "i")


conv_spike_matrix = function(sp_m, w) {
    t(sapply(1:nrow(sp_m), function(i) filter(sp_m[i,], w, circular=TRUE)))
}

cut_window_mat = function(i,x) {
    w_i = rev((i-L+1):i)
    w_i = w_i[w_i>0]
    
    return(matrix(x[,w_i], nrow=M, ncol=length(w_i)))
}

conv_mat = Vectorize(function(i, y, w) {
    yc = cut_window_mat(i,y)
    wc = matrix(w[1:length(yc)], nrow=M, ncol=ncol(yc))
    
    sum(sapply(1:M, function(i) t(wc[i, ]) %*% yc[i,]))
}, "i")

