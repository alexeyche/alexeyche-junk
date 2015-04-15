

require(Rdnn)
require(rjson)

setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")

source('plot_stat.R')
source('gen_poisson.R')

const = "/home/alexeyche/cpp/const.json"
cr = fromJSON(parseConst(const))

cjson = toJSON(cr)

const = RConstants$new(cjson)
s = RSim$new(const)

len = 10000
t = seq(1,len)/1000
Iin = 0.0 +0.8*cos(2*pi*3.5*t + 0.8)
RProto$new("/home/alexeyche/cpp/build/input.pb")$write(list(values=Iin), "TimeSeries")


s$setTimeSeries(Iin)
s$run(4)

stat = s$getStat()
net = s$getSpikes()

sim_rate = 1000*length(net[[1]])/10000.0
prast(net,T0=1000, Tmax=2000)



get_st = function(stat, name) {
    X = NULL
    for(st in names(stat)) {
        if(grepl(sprintf("^%s", name), st)) {
            X = rbind(X, stat[[st]])       
        }
    }
    return(X)
}
# 
# par(mfrow=c(3,1))
# plotl(stat[[1]][["Stdp_y"]][1:1000])
# plotl(stat[[1]][["Stdp_x1"]][1:1000])
# plotl(stat[[1]][["Stdp_w1"]][1:1000])
# plot_st(stat[[1]], "Stdp_w")


