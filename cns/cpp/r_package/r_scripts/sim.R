

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

s$setTimeSeries(c(1,1,1,1,1,1))
s$run(4)
stat = s$getStat()

