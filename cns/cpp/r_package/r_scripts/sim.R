

require(Rsnn)
require(rjson)
setwd("~/prog/alexeyche-junk/cns/cpp/r_package/r_scripts")

source('plot_stat.R')
source('gen_poisson.R')
const = "/home/alexeyche/prog//alexeyche-junk/cns/cpp/const.json"


cr = fromJSON(parseConst(const))

cr$sim_configuration$net_layers_conf[[1]]$size = 100
cr$sim_configuration$net_layers_conf[[1]]$neuron_conf$learning_rule = "OptimalStdp"
cr$sim_configuration$conn_map[["0->1"]][[1]]$weight_distr = "Norm(1.5,0.05)"
cr$sim_configuration$neurons_to_listen = c(100, 151)
#cr$sim_configuration$sim_run_conf$seed = 1
#cr$learning_rules$Stdp$learning_rate = 0.75
#cr$learning_rules$Stdp$a_minus = 5
cr$learning_rules$OptimalStdp$learning_rate = 0.04
cr$learning_rules$OptimalStdp$weight_decay = 0.04
cjson = toJSON(cr)

const = RConstants$new(cjson)

s = RSim$new(const, jobs=4)

rate = 5
len = 10000
N = cr$sim_configuration$input_layers_conf[[1]]$size

set.seed(1)

input_net = gen_correlated_poisson(N, mean_rate=rate, group_size=2)

sp = list(spikes_list=input_net, labels=c("1"), timeline=c(len))

s$setInputSpikesList(sp)
#s$collectStat()

pic_dir = tempdir()
for(ep in 1:16) {
    s$run()
    net = s$getSpikes()
    model = s$getModel()
    
    pic_file = sprintf("%s/%s.png", pic_dir, ep)
    png(pic_file, width=1224, height=868)
    p1 = prast(net,T0=0,Tmax=1000, lab=sprintf("epoch %s", ep))
    p2 = levelplot(t(model[["w"]]), col.regions=colorRampPalette(c("black", "white")))
    
    print(p1, position=c(0, 0.7, 1, 1), more=TRUE)
    print(p2, position=c(0, 0, 1, 0.7))
    
    invisible(dev.off())
    system(sprintf("( eog -w %s 2>&1 1>/dev/null )&", pic_file))   
    
    stat = s$getStat()
    if(length(stat)>0) {
        pic_file = sprintf("%s/%s_stat.png", pic_dir, ep)
        png(pic_file, width=1224, height=868)
        plot_stat(stat, cr, net, model, 2, syn_id=51)
        
        invisible(dev.off())
        if(file.exists(pic_file)) {
            system(sprintf("( eog -w %s 2>&1 1>/dev/null )&", pic_file))
        }
    }
}
