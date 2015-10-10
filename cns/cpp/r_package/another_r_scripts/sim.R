
require(Rdnn)
require(rjson)

setwd(path.expand("~/dnn/r_scripts"))

source("env.R")
source("ucr_ts.R")
source("ucr_data_to_spikes.R")
source("gen_poisson.R")

cr = fromJSON(readConst(CONST_JSON))
cr$sim_configuration$layers[[2]][[ 
    which(names(cr$sim_configuration$layers[[2]]) == "learning_rule")  
]] <- NULL
cr$sim_configuration$layers[[2]][[ 
    which(names(cr$sim_configuration$layers[[2]]) == "weight_normalization")  
]] <- NULL
cr$sim_configuration$neurons_to_listen = c(50)
cr$sim_configuration$seed = 1
set.seed(1)
cjson = toJSON(cr)

const = RConstants$new(cjson)



N = cr$sim_configuration$layers[[1]]$size

rates = seq(1, 100, length.out=100)
out_rates = NULL
for(rate in rates) {
    len = 10000
    spikes = gen_poisson(N, rate, len)
    
    s = RSim$new(const)    
    #s$turnOnStatistics()
    s$setInputSpikes(spikes, "SpikeSequenceNeuron")

    s$run(1)

    #stat = s$getStat()
    net = s$getSpikes()
    #m = s$getModel()

    out_rate = mean(1000*sapply(net$values[51:99], length)/len)
    out_rates = c(out_rates, out_rate)
}

#prast(net$values,T0=0,Tmax=1000)
#plot_stat(stat[[1]], 1)



