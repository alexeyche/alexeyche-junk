
net = list()
net[gr1$ids] = gr1$patterns[[1]]$data
net[neurons$ids] = -Inf

net_neurons = list(neurons)

c(net, net_neurons, stat, mean_grad) := run_srm(net_neurons, net, run_options)
int_options = list(T0=0, Tmax=100, dim=sum(sapply(neurons$id_conns, length)), quad=256)
int_out = integrateSRM_vec(constants, int_options, neurons$ids, neurons$id_conns, neurons$weights, net)

#syn_epsp = sapply(net, function(sp) sum(epsp(t-sp)))  
#ps = p_stroke(neurons$u(t, net))
#old_out = lapply(1:neurons$len, function(id) syn_epsp[neurons$id_conns[[id]] ] * ps[id] )  
grad = adaptIntegrate(function(x) integrand(x, net, neurons), 0, 100, tol=1e-06, fDim=sum(sapply(neurons$id_conns, length)), maxEval=1000)$integral
sum(int_out$out-grad)

