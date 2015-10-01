
require(Rdnn)
require(MASS)

par(mfrow=c(2,2))
Nsamples = 100


data = NULL
data = rbind(data, mvrnorm(
    Nsamples
  , Sigma=diag(2)*0.75
  , mu=c(2,2)
))
data = rbind(data, mvrnorm(
    Nsamples
    , Sigma=diag(2)*0.75
    , mu=c(0,3)
))
data = rbind(data, mvrnorm(
    Nsamples
    , Sigma=diag(2)*0.75
    , mu=c(2,5)
))
rownames(data) <- c(rep("1", Nsamples), rep("2", Nsamples), rep("3", Nsamples))

data = data[ sample(nrow(data)), ]

d = lda(x=data, grouping=as.factor(rownames(data)))
dv = predict(d, data)

K = data %*% t(data)
c(y, M, N, A) := KFD(K)

metrics_str = sprintf("%f, %f", tr(M)/tr(N), tr(A))

ans = K %*% y[, 1:2]

plot(data, col=as.integer(rownames(data)))
plot(dv$x[,1], col=as.integer(rownames(data)))
plot(Re(ans[,1]), col=as.integer(rownames(data)), main=metrics_str) # eureka!
plot(Re(ans), col=as.integer(rownames(data))) # eurica!
