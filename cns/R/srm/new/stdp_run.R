setwd("~/my/git/alexeyche-junk/cns/R/srm/new")

source('constants.R')
if(refr_mode == 'low') {
  u_abs <- -120 # mV
  u_r <- -50#-50 # mV
  trf <- 2.25 # ms
  trs <- 2 # ms
  dr <- 1 # ms
} else 
  if(refr_mode == 'middle') {
    u_abs <- -150 # mV
    u_r <- -50#-50 # mV
    trf <- 3.25 # ms
    trs <- 3 # ms
    dr <- 1 # ms
  } else
    if(refr_mode == 'high') {
      u_abs <- -250 # mV
      u_r <- -70#-50 # mV
      trf <- 5.25 # ms
      trs <- 5 # ms
      dr <- 3 # ms
    }

source('util.R')
source('plot_funcs.R')
source('ucr_ts.R')
source('gen_spikes.R')
source('neuron.R')

source('target_functions.R')
source('learn_and_run_net.R')
source('srm.R')
source('grad_funcs.R')
source('serialize_to_bin.R')
source('eval_funcs.R')
source('layers.R')
source('kernel.R')


ID_MAX=0

data = synth # synthetic control

set.seed(1234)
c(train_dataset, test_dataset) := read_ts_file(data,'~/my/sim')
elems = 1
train_dataset = train_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150,elems),
                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150, elems),
                              sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]


N = 50
M = 50
dt = 0.5

start_w.M = matrix(rnorm( M*N, mean=20, sd=0.5), ncol=N, nrow=M)
start_w.N = matrix(rnorm( (N-1)*N, mean=20, sd=0.5), ncol=N, nrow=(N-1))

gr1 = TSNeurons(M = M)
gr2 = TSNeurons(M = M, ids_c = 1000:(1000+M))
gr1$loadPatterns(train_dataset, duration, dt, lambda=5)
gr2$loadPatterns(test_dataset, duration, dt, lambda=5)
patt_len = length(gr1$patterns)


inhib_frac = 0.25
neurons = SRMLayer(N, start_w.N, p_edge_prob=edge_prob, ninh=ceiling(N*inhib_frac))

connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
ninput_n = N/2
nnet_conn = M/5
for(i in 1:ninput_n) {
  cc = sample(gr1$ids, M-nnet_conn)
  connection[cc,i] = 0
}
connection[,(ninput_n+1):N] = 0

neurons$connectFF(connection, start_w.M, 1:N )

net = list()
net[gr1$ids] = gr1$patterns[[1]]$data
net[neurons$ids] = -Inf

uu = NULL
pp = NULL
for( t in seq(0, duration ,by=dt)) {
  u = USRM(t, constants, neurons$ids[1], neurons$id_conns[[1]], neurons$weights[[1]], net)
  uu=c(uu, u)
  if(g(u)*dt > runif(1)) {
    net[[ neurons$ids[1] ]] = c(net[[ neurons$ids[1] ]] , t)
  }
  pp=c(pp, g(u))
}
