

require(Rdnn)
require(rjson)

setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")

source('plot_stat.R')
source('gen_poisson.R')

const = "/home/alexeyche/cpp/const.json"
cr = fromJSON(parseConst(const))

cr$sim_configuration$net_layers_conf[[1]]$size = 100
cjson = toJSON(cr)

const = RConstants$new(cjson)
s = RSim$new(const)

x = 6.5 + 6.5 * sin(seq(1,1000)/10)
RProto$new("/home/alexeyche/cpp/build/input.pb")$write(list(values=x), "TimeSeries")

s$setTimeSeries(x)
s$run(4)
stat = s$getStat()
net = s$getSpikes()
prast(net)
plotl(stat[[1]][[1]])
