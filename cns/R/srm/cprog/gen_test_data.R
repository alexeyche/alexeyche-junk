#!/usr/bin/RScript

library(snn)
setwd("~/prog/alexeyche-junk/cns/R/srm/cprog")
#setwd("~/my/git/alexeyche-junk/cns/R/srm/cprog")



spike_file = "/home/alexeyche/prog/sim/test_spikes"

N=5

gen_pattern = function(f, dur, N, len=400, del=70) {
    net=blank_net(N)
    for(t in seq(0,dur,length.out=len)) {
        i = as.integer((N-1)*(1+f(t/del))/2)+1
        net[[i]] = c(net[[i]], t) 
    }
    return(net)
}
fun1 = function(x) {
    return(cos(x))
}
fun2 = function(x) {
    return(-cos(x))
}
patt_dur = 1000
p1 = gen_pattern(fun1, patt_dur, N)
p2 = gen_pattern(fun2, patt_dur, N)

patt_nums = 60
net = blank_net(N)
patts = list(p1, p2)
glob_t = 0
classes = c()
timeline = c()
for(i in 1:patt_nums) {
    p_id = sample(length(patts),1)
    for(ni in 1:N) {
        net[[ni]] = c(net[[ni]], patts[[p_id]][[ni]] + glob_t)
    }
    glob_t = glob_t + patt_dur
    timeline = c(timeline, glob_t)
    classes = c(classes, as.double(p_id))
}

plot_rastl(p1)

saveMatrixList(spike_file, list(list_to_matrix(net), 
                                matrix(timeline),
                                matrix(classes)
)
)

sp = loadMatrix("~/prog/sim/output_spikes",1)
neto = getSpikesFromMatrix(sp)
plot_rastl(neto,T0=0,Tmax=1000)

sf="~/prog/sim/test_stat"
p =loadMatrix(sf, 1)
u =loadMatrix(sf, 2)
syn =loadMatrix(sf, 3)
W = loadMatrix(sf, 3+100)

p2 = loadMatrix(sf, 3+10+10+1)
C2 = loadMatrix(sf, 3+10+10+3+10+1)

plotl(p[1,])

plotl(u[1,])
plotl(B[1,])
plotl(C1[5,])
plotl(W1[5,])



mf ="~/prog/sim/test_model"
W = loadMatrix(mf,1)
