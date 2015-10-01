
require(Rsnn)

len = 1000

x = sin(1:len/10.0)


L = 100

f = rnorm(L)

s = NULL
for(i in 1:(len-L)) {
    s = c(s, f %*% x[i:(i+L-1)])
}

t_i = 1:L
