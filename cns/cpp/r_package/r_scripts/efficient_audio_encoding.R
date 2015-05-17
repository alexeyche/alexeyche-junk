require(R.matlab)
require(waved)
require(Rdnn)

riken_dir = sprintf("%s/dnn/datasets/riken", path.expand("~"))
data_14chan = sprintf("%s/SubC_14chan_3LRR.mat", riken_dir)

#data = readMat(data_14chan)


max_val = max(data$EEGDATA)
min_val = min(data$EEGDATA)
normalize = function(x, min_val, max_val) {
    1 - (max_val-x)/(max_val-min_val) 
}
norm = function(x) {
    x/sqrt(sum(x^2))
}

chan = 1
ex = 1
xt = data$EEGDATA[chan,,ex]


xt = norm(xt)

M = 100
L = 100

set.seed(1)
filters = matrix(rnorm(M*L), nrow=L, ncol=M)
filters = norm(filters)

fi = filters
fi = norm(fi)

i = 1 
s_thr = 0.1

spikes = list()

for(i in 1:(length(xt)-L)) {
    x_start = xt[i:(i+L-1)]
    x = x_start
    
    s_v = NULL
    si_v = NULL
    for(j in 1:200) {
         ip = c(x %*% t(fi))
        si = which(ip == max(ip))
        s = ip[si]
        res = x - s * fi[si,]    
        cat("s: ", s, ", si: ", si, ", |res|: ", sqrt(sum(res^2)), "\n")
        if(s>=s_thr) {
            cat("Spike!")
            spikes[[length(spikes)+1]] = list(t=i,si=si,s=s)
        }
        x = res
        s_v = c(s_v, s)
        si_v = c(si_v, si)
    }
    
    x_d = colSums(s_v*fi[si_v,])
    
    delta = x_start - x_d
    for(s_i in 1:length(si_v)) {
        s = s_v[s_i]
        si = si_v[s_i]
        grad = 1*s*delta
        fi[si, ] = fi[si, ] + grad    
   }   

}
    
plot(x_start, ylim=c(min(x,x_start), max(x,x_start)), type="l")
lines(x_d,col="red")
x = c()
y = c()
cex = c()
for(sp in spikes) {
    x = c(x, sp$t)
    y=c(y, sp$si)
    cex = c(cex, sp$s)
    
}

xyplot(y ~ x, list(x = x, y = y), cex=cex*10,  col = "black")