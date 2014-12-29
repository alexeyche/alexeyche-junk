
require(Rsnn)

setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")
source('gen_poisson.R')

N = 1
len = 100000
T = 100000

x = seq(0, T, length.out=len)
net = list()
net_b = NULL
for(ni in 1:N) {
    f1 = sin(x/100)
    a = sample(seq(0.1,10, length.out=100), 1)
    b = sample(seq(-10,10, length.out=100), 1)
    f2 = sin(a*x/100-b)
    
    f = list(f1, f2)
    f_sp = matrix(0, nrow=length(f), ncol=len)
    
    for(fi in 1:length(f)) {
        dd = diff(which(diff(f[[fi]])>0))
        d_fired = dd != 1
        f_sp[fi, d_fired] <- 1
        f_sp[fi, !d_fired] <- 0  
    }
    f_sp = colSums(f_sp)
    net[[ni]] = which(f_sp >= 1)
    net_b = rbind(net_b, f_sp)
}
