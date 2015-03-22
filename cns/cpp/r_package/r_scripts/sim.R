

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

len = 1000
x = (1.0 + sin(seq(1,len)/10))*0.25
RProto$new("/home/alexeyche/cpp/build/input.pb")$write(list(values=x), "TimeSeries")

s$setTimeSeries(x)
s$run(4)

stat = s$getStat()
net = s$getSpikes()
prast(net,T0=0,Tmax=1000)
#plotl(stat[[1]][[1]])

plot_st =function(stat, name) {
    X = NULL
    for(st in names(stat)) {
        if(grepl(sprintf("^%s", name), st)) {
            X = rbind(X, stat[[st]])       
        }
    }
    plotl(colMeans(X))
}
