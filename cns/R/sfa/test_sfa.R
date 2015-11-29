
require(rSFA)

require(Rdnn)


# type = c("Normal", "Aggressive")
# norm_actions = c("Bowing","Clapping","Handshaking","Hugging","Jumping","Running","Seating","Standing","Walking","Waving")
# aggr_actions = c("Elbowing","Frontkicking","Hamering","Headering","Kneeing","Pulling","Punching","Pushing","Sidekicking","Slapping")
# subs = c("sub1", "sub2", "sub3", "sub4")
# 
# f = ds.path("emg_active", subs[1], type[1], "txt", paste(norm_actions[1], ".txt", sep=""))
# 
# rdata = as.matrix(read.table(f))
# 
# dm = colMeans(rdata)
# data = (dm - rdata)/sqrt(colMeans((rdata-dm)^2))


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




t = seq(0,2*pi, length.out=1000)
x1 = sin(t) + cos(11*t)^2
x2 = cos(11*t)

xr = matrix(c(x1, x2), ncol=2, nrow=1000)


xn = norm(xr)

h = NULL
for(i in 1:ncol(xn)) {
    h = cbind(h, xn[, i])
    for(j in 1:ncol(xn)) {
        h = cbind(h, xn[, i]*xn[, j])
    }
}


hc = centering(h)
z = whitening(hc)

dz = NULL
for(i in 1:ncol(z)) {
    dz = cbind(dz, diff(z[,i]))
}

prc = eigen(t(dz) %*% dz)
lambda_i = order(prc$values, decreasing=FALSE)

g = NULL
for(i in lambda_i) {
    g = cbind(g, t(t(prc$vectors[,i]) %*% t(z)))    
}


