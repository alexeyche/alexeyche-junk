

require(Rsnn)
require(rjson)
setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")
source('plot_stat.R')

const = "/home/alexeyche/prog//alexeyche-junk/cns/cpp/const.json"


cr = fromJSON(parseConst(const), unexpected.escape ="skip")

cr$sim_configuration$net_layers_conf[[1]]$size = 100
cr$sim_configuration$net_layers_conf[[1]]$neuron_conf$learning_rule = "Stdp"
cr$sim_configuration$conn_map[["0->1"]][[1]]$weight_distr = "Norm(1.5,0.05)"
cr$sim_configuration$neurons_to_listen = c(100, 151)
#cr$sim_configuration$sim_run_conf$seed = 1
cr$learning_rules$Stdp$learning_rate = 2

cjson = toJSON(cr)

const = RConstants$new(cjson)

s = RSim$new(const, jobs=8)

rate = 5
len = 10000
N = cr$sim_configuration$input_layers_conf[[1]]$size

set.seed(1)

input_net = lapply(1:N, function(x) which(rpois(len, rate/1000) == 1))

sp = list(spikes_list=input_net, labels=c("1"), timeline=c(len))

s$setInputSpikesList(sp)
#s$collectStat()

for(ep in 1:10) {
    s$run()
    net = s$getSpikes()
    model = s$getModel()
    
    p1 = prast(net,T0=0,Tmax=1000)
    p2 = levelplot(t(model[["w"]]), col.regions=colorRampPalette(c("black", "white")))
    
    print(p1, position=c(0, 0.7, 1, 1), more=TRUE)
    print(p2, position=c(0, 0, 1, 0.7))
    
    stat = s$getStat()
    plot_stat(stat, cr, net, model, 2, syn_id=30)
}
    

