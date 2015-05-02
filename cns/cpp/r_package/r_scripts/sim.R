
require(Rdnn)
require(rjson)

setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")

source('plot_stat.R')

const = "/home/alexeyche/cpp/const.json"
cr = fromJSON(readConst(const))

cjson = toJSON(cr)

const = RConstants$new(cjson)
s = RSim$new(const)

# len = 10000
# t = seq(1,len)/1000
# Iin = 0.0 +0.8*cos(2*pi*3.5*t + 0.8)
# RProto$new("/home/alexeyche/cpp/build/input.pb")$write(list(values=Iin), "TimeSeries")

source("./ucr_data_to_spikes.R")
spikes = spikes_complect[["train"]]$values
spikes = sapply(spikes_complect[["train"]]$values, function(x) x[x<10000])
s$setInputSpikes(spikes, "SpikeSequenceNeuron")

s$run(1)

stat = s$getStat()
net = s$getSpikes()
m = s$getModel()

prast(net,T0=0,Tmax=10000)

lsize = sapply(cr$sim_configuration$layers, function(x) x$size)
w = m[["w"]]
maps = getWeightMaps(5,5, w, lsize)
#plotl(maps[[2]][5,])
#gr_pl(t(w))

#plotl(get_st(stat[[1]], "u")[1:1000])
#plotl(get_st(stat[[1]], "StaticSynapse_x100")[1000:2000])
#plotl(stat[[6]][[1]][1:1000])
#y = get_st(stat[[1]],"Stdp_y")
#x = get_st(stat[[1]], "Stdp_x")
#wl = get_st(stat[[1]], "Stdp_w")

# par(mfrow=c(3,1))
# plot(x[2,0:2200], type="l", ylab="x")
# plot(y[1600:2200], type="l", ylab="y")
# plot(wl[3,1600:2200], type="l", ylab="w")

#plotl(colMeans(get_st(stat[[1]], "Stdp_w")))
s$saveModel("/home/alexeyche/cpp/build/model.pb")


