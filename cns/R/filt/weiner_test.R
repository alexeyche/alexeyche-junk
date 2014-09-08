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

require(MASS)

Lx = 10000

L=2

x = sin(2*pi/8*(seq(0,Lx-1)))

r = xcorr(x,len=L-1)/Lx 
R = toeplitz(r[L:1], r[L:(2*L-1)])

cf=c(1,-1)

d = filter(x, cf, circular=TRUE)

rdx = xcorr(d, x, L-1)/Lx
p = rdx[L:(2*L-1)]

w_opt = ginv(R) %*% p

xs = filter(d, rev(w_opt), circular=TRUE)

hist(x-xs)
