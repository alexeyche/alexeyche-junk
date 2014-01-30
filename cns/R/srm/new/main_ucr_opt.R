#setwd("~/prog/alexeyche-junk/cns/R/srm/new")
#dir = '~/prog/sim/spear_run'

setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
dir = '~/my/sim/spear_run'
source('constants.R')

args <- commandArgs(trailingOnly = TRUE)

name = args[1]
dir = sprintf("%s/%s", dir, name)
system(sprintf("mkdir %s",dir))

alpha = as.numeric(args[2])
beta = as.numeric(args[3])
lr = as.numeric(args[4])
llh_depr = as.numeric(args[5])
refr_mode = as.character(args[6])

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

data = synth # synthetic control

set.seed(1234)
c(train_dataset, test_dataset) := read_ts_file(data, '~/my/sim')
elems = 2
train_dataset = train_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150,elems),
                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150, elems),
                              sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]


dt = 0.5

gr1 = TSNeurons(M = M)
gr2 = TSNeurons(M = M, ids_c = 1000:(1000+M))
neurons = SRMLayer(N, start_w.N, p_edge_prob=edge_prob)

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

test_trials=3

run_options = list(T0 = 0, Tmax = duration, dt = dt, 
                   learning_rate = lr, epochs = 10, start_epoch = 1, weight_decay = 0,
                   reward_learning=FALSE,
                   fp_window = 30, fp_kernel_size = 15, dev_frac_norm = 0.25,
                   learn_window_size = 150, mode='learn', collect_stat=TRUE, 
                   target_set = list(depress_null=FALSE),
                   learn_layer_id = 1,
                   test_patterns = gr2$patterns, 
                   test_function = function(train_set, test_set) {
                     sigma = 10
                     window = 10
                     
                     k_out_train = lapply(train_set, function(st) kernelPass_spikes(st, list(sigma = sigma, window = window, T0 = 0, Tmax = duration, quad = 256)) )
                     k_out_test = lapply(test_set, function(st) kernelPass_spikes(st, list(sigma = sigma, window = window, T0 = 0, Tmax = duration, quad = 256)) )

                     perf = ucr_test(mean_on_trials(k_out_train, test_trials), mean_on_trials(k_out_test, test_trials), eucl_dist_alg, verbose=FALSE)
                     return( list(loss=perf$rate))
                   }, evalTrial=test_trials, test_run_freq=2
)

layers = SimLayers( list(neurons) )
input_neurons = gr1

loss = run_net(gr1, layers, run_options, verbose=FALSE)
cat(min(loss), "\n")


