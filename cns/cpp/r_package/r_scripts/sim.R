
require(Rdnn)
require(rjson)

setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")

source('plot_stat.R')

const = "/home/alexeyche/cpp/const.json"
cr = fromJSON(parseConst(const))

cjson = toJSON(cr)

const = RConstants$new(cjson)
s = RSim$new(const)

len = 10000
t = seq(1,len)/1000
Iin = 0.0 +0.8*cos(2*pi*3.5*t + 0.8)
RProto$new("/home/alexeyche/cpp/build/input.pb")$write(list(values=Iin), "TimeSeries")

source("./ucr_data_to_spikes.R")
spikes = spikes_complect[["train"]]$values
#spikes = sapply(spikes_complect[["train"]]$values, function(x) x[x<10000])
s$setInputSpikes(spikes, "SpikeSequenceNeuron")

s$run(4)

stat = s$getStat()
net = s$getSpikes()
m = s$getModel()

prast(net,i=102, plen=400)

lsize = sapply(cr$sim_configuration$layers, function(x) x$size)
w = m[["w"]]
maps = getWeightMaps(5,5, w, lsize)
#plotl(maps[[2]][5,])
#gr_pl(t(w))

#plotl(get_st(stat[[6]], "u")[1:100])
#plotl(stat[[6]][[1]][1:1000])


s$saveModel("/home/alexeyche/cpp/build/model.pb")


