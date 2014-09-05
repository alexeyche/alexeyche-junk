

H = matrix(rnorm(200), nrow=2, ncol=100)

R = matrix(rnorm(200), nrow=2, ncol=100)

s1 = convolve(H[1,],R[1,])
s2 = convolve(H[2,],R[2,])

rowSums(t(H) %*% R)