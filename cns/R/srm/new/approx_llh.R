#!/usr/bin/RScript

setwd("~/my/git/alexeyche-junk/cns/R/srm/new")

source('util.R')
source('neuron.R')
source('layers.R')
source('gen_spikes.R')
source('plot_funcs.R')
source('grad_funcs.R')
source('llh.R')
source('srm.R')
source('kernel.R')

ID_MAX=0

M = 10
N = 1
duration = 150
dt = 0.5

gr1 = TSNeurons(M = M)

gr1$loadPattern( 1:10, duration, 1, dt)

start_w = 3.0

neurons = SRMLayer(N, start_w)
neurons$connectFF(gr1$ids, start_w)

net = list()

epochs = 10
ro = run_options = list(T0 = 0, Tmax = 150, dt = dt, learning_rate = 0.5, learn_window_size = 150, 
                   mode="run", collect_stat=TRUE, learn_layer_id = 1, target_set = list(depress_null=FALSE), weight_decay=0)
layers = SimLayers(list(neurons))

grads = list()
net_all = list()


for(ep in 1:epochs) {
    net[neurons$ids] = -Inf
    net[gr1$ids] = gr1$patterns[[1]]$data
    
    c(net, layers, stat, grad) := run_srm(layers, net, run_options)
    
    grad = layers$l[[1]]$grad(0, duration, net, ro$target_set)

    grads[[ep]] = grad
    net_all[[ep]] = net[[ neurons$ids[1] ]]
}
