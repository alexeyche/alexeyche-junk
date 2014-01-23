setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
source('gen_spikes.R')
source('neuron.R')
source('util.R')
source('ucr_ts.R')


dir = '~/my/sim'
data = synth # synthetic control

set.seed(1234)
c(train_dataset, test_dataset) := read_ts_file(data)
train_dataset = train_dataset[c(sample(1:50, 10), sample(51:100, 10), sample(101:150,10),
                                sample(151:200, 10), sample(201:250,10), sample(251:300,10))] # cut
test_dataset = test_dataset[c(sample(1:50, 10), sample(51:100, 10), sample(101:150, 10),
                              sample(151:200, 10), sample(201:250,5), sample(251:300, 10))]

ucr_test(train_dataset, test_dataset, eucl_dist_alg)
  


M=50
N=10
start_w.N = 5
start_w.M = 10
duration = 300
dt=0.5

gr1 = TSNeurons(M = M)
gr2 = TSNeurons(M = M, ids_c = 1000:(1000+M))
neurons = SRMLayer(N, start_w.N, p_edge_prob=0.5)

gr1$loadPatterns(train_dataset, duration, dt, lambda=5)
gr2$loadPatterns(test_dataset, duration, dt, lambda=5)
patt_len = length(gr1$patterns)
gr1$patterns = gr1$patterns[sample(patt_len)]
#plot_rastl(gr1$patterns[[3]]$data)

connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
connect_window = N*2
step = M/N
overlap = 1
for(ni in 0:(N-1)) {
  if(ni != 0) connection[1:((ni*step)-overlap),ni+1] = 0    
  if(ni != N-1) connection[((ni*step)+step+1+overlap):M,ni+1] = 0
}

neurons$connectFF(connection, start_w.M, 1:N )


SimLayers = setRefClass("SimLayers", fields=list(layers="list", sim_options="list"), methods=list(
                    initialize = function(T0, Tmax, dt, layers_c=NULL) {
                      sim_options <<- list(T0 = T0, Tmax = Tmax, dt = dt, saveStat = FALSE)
                      if(!is.null(layers_c)) 
                        layers <<- layers_c
                    },
                    sim = function(net) {
                      simLayers(.self$sim_options, constants, .self$layers, net)
                    }))

sl = SimLayers(0, 300, 0.5, list(neurons))

net = list()
net[gr1$ids] = gr1$patterns[[1]]$data
net[neurons$ids] = -Inf

sim_out = sl$sim(net)
