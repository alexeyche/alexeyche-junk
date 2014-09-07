nextpow2 <- function(x) {
    if (is.null(x) || length(x) == 0) return(c())
    if (!is.numeric(x) && !is.complex(x))
        stop("Argument 'x' must be a numeric/complex vector/matrix.")
    
    x[x == 0] <- 1
    return(ceiling(log2(abs(x))))
}


Lx = 10000

L=6

x = sin(2*pi/8*(seq(0,Lx-1)))

cf=c(1,-1)

d = filter(x, cf)

rxx = acf(x, lag.max=(L-1), plot=FALSE)

x = seq(0,10, length.out=100)


len = 4
nfft = 2^nextpow2(2*len-1)

rxx = fft(fft(x,nfft) * Conj(fft(x,nfft)), inverse=TRUE)


rxx = c(rxx[(length(rxx)-len+2):length(rxx)], rxx[1:len]) 