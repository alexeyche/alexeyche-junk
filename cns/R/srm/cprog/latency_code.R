
library(snn)
tsf = "/home/alexeyche/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_60"

ts = loadMatrix(tsf,1)
maxval = max(ts)
minval = min(ts)

ts = (ts-minval)/(maxval-minval)

Tmax = 10
alpha = 10

getT = Vectorize(function(s) {
    Tmax - log(alpha*s+1)
})


sp = getT(ts)
plot(sp, rep(1,60))