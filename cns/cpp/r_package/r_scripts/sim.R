

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
Iin = 1.0 +0.8*cos(2*pi*3.5*t + 0.8)
#I0 = 1.5
#Iin = sapply(1:length(t), function(i) I0)
RProto$new("/home/alexeyche/cpp/build/input.pb")$write(list(values=Iin), "TimeSeries")

# act_f = cr$sim_configuration$layers[[1]]$act_function
# tr = cr$act_functions[[act_f]]$treshold
# liaf_c = cr$neurons$LeakyIntegrateAndFire
# tau_m = 1/liaf_c$gL
# R = tau_m/liaf_c$C
# rate = 1000*(liaf_c$tau_ref + tau_m*log(R*I0/(R*I0 - (tr-liaf_c$rest_pot)) ))^(-1)




s$setTimeSeries(Iin)
s$run(4)

stat = s$getStat()
net = s$getSpikes()

sim_rate = 1000*length(net[[1]])/10000.0
prast(net,T0=0, Tmax=1000)


plotl(stat[[1]][["u"]][1:1000])

# dtsp = 1/(sim_rate/1000)
# liaf_c$rest_pot + R*I0 *(1-exp(-dtsp/tau_m))



#plot_st(stat[[1]],"x")
#plotl(stat[[2]][["Ca"]][1:1000])
