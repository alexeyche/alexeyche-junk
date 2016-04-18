
require(rSFA)

require(Rdnn)



norm = function(x) {
    xm = colMeans(x)
    r = NULL
    for(i in 1:ncol(x)) {
        xd = x[, i] - mean(x[, i])
        r = cbind(r, xd/sqrt(mean(xd^2)))
    }
    return(r)
}
whitening = function(x) {
    r = svd(hc)
    return(r$u %*% r$v)
}
centering = function(x) {
    xm = colMeans(x)
    r = NULL
    for(i in 1:ncol(x)) {
        r = cbind(r, x[, i] - xm[i])
    }
    return(r)
}
expansion = function(x) {
    h = NULL
    for(i in 1:ncol(x)) {
        h = cbind(h, x[, i])
        for(j in 1:i) {
            h = cbind(h, x[, i]*x[, j])
        }
    }
    return(h)
}



t = seq(0,2*pi, length.out=1000)
x1 = sin(t) + cos(11*t)^2
x2 = cos(11*t)

xr = matrix(c(x1, x2), ncol=2, nrow=1000)

sp = spikes
sp$values = spikes$values[101:200]
spikes.pr = preprocess.run(Epsp(TauDecay=50, Length=200), binarize.spikes(spikes), 4)
xr = t(spikes.pr$values)
xn = norm(xr)
#h = expansion(xn)
h = xn
hc = centering(h)
z = whitening(hc)

dz = NULL
for(i in 1:ncol(z)) {
    dz = cbind(dz, diff(z[,i]))
}

prc = eigen(t(dz) %*% dz)
lambda_i = order(prc$values, decreasing=FALSE)

lambda_i = lambda_i[1:2]

g = NULL
for(i in lambda_i) {
    g = cbind(g, t(t(prc$vectors[,i]) %*% t(z)))
}

K = kernel.run(Dot(), time.series(t(g), spikes.pr$info), 8)

c(y, M, N, A) := KFD(K)
c(metric, K, y, M, N, A) := list(-tr(M)/tr(N), K, y, M, N, A)

ans = K %*% y[, 1:2]

par(mfrow=c(1,2))

metrics_str = sprintf("%f", metric)
plot(Re(ans[,1]), col=as.integer(rownames(K)), main=metrics_str) 
plot(Re(ans), col=as.integer(rownames(K)))        


ts.sfa = chop.time.series(time.series(t(g[,1:2]), spikes.pr$info))