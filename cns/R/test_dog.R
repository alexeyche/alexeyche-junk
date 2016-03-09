
l = 100
ids = 1:l

w = matrix(0, nrow=l, ncol=l)

gf = function(x, xc, sigma) exp( - ( (x-xc)*(x-xc) )/(2*sigma*sigma) )

for(i in ids) {
    for(j in ids) {
        w[i, j] = gf(i,j, 5.0)
        w[i, j] = w[i, j] + gf(i,j-l, 5.0)
        w[i, j] = w[i, j] + gf(i,j+l, 5.0)
    }
}


gr_pl(w)