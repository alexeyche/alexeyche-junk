#setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
setwd("~/prog/alexeyche-junk/cns/R/srm/new")

dir = '~/prog/sim/test3_simp'


system(sprintf("find %s -maxdepth 1 -name \"*.png\" -type f -exec rm -f {} \\;", dir))

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
c(train_dataset, test_dataset) := read_ts_file(data,'~/prog/sim')
elems = 1
train_dataset = train_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150,elems),
                                sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150, elems),
                              sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]


train_dataset = train_dataset[rep(1, 10)]
perf = ucr_test(train_dataset, test_dataset, eucl_dist_alg)
cat("baseline:", perf$rate, "\n")

N = 50
M = 50
dt = 0.5

start_w.M = matrix(rnorm( M*N, mean=10, sd=0.5), ncol=N, nrow=M)
start_w.N = matrix(rnorm( (N-1)*N, mean=10, sd=0.5), ncol=N, nrow=(N-1))


gr1 = TSNeurons(M = M)
gr2 = TSNeurons(M = M, ids_c = 1000:(1000+M))


inhib_frac = 0.5
neurons = SRMLayer(N, start_w.N, p_edge_prob=edge_prob, ninh=ceiling(N*inhib_frac))

gr1$loadPatterns(train_dataset, duration, dt, lambda=5)
gr2$loadPatterns(test_dataset, duration, dt, lambda=5)
patt_len = length(gr1$patterns)
#gr1$patterns = gr1$patterns[sample(patt_len)]
#plot_rastl(gr1$patterns[[3]]$data)

connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
ninput_n = N/2
nnet_conn = M/5
for(i in 1:ninput_n) {
  cc = sample(gr1$ids, M-nnet_conn)
  connection[cc,i] = 0
}
connection[,(ninput_n+1):N] = 0

neurons$connectFF(connection, start_w.M, 1:N )

runmode="learn"
#runmode="run"
test_trials=1

run_options = list(T0 = 0, Tmax = duration, dt = dt, 
                   learning_rate = lr, epochs = 100, start_epoch = 1, weight_decay = 0,
                   reward_learning=FALSE,
                   fp_window = 30, fp_kernel_size = 15, dev_frac_norm = 0.25,
                   learn_window_size = duration/2, mode=runmode, collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_4spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1,
                   test_patterns = gr2$patterns, 
                   test_function = function(train_set, test_set) {
                     vall = NULL
                     rate = NULL
                     for(window in c(8,10, 12, 14, 16, 20)) {
                       for(sigma in c(2,3,4, 6, 8, 10)) {
                         
                          k_out_train = lapply(train_set, function(st) kernelPass_spikes(st, list(sigma = sigma, window = window, T0 = 0, Tmax = duration, quad = 256)) )
                          s = diffTrials(k_out_train, length(k_out_train))
                          cat("win: ", window, " sig: ", sigma, " diff: ", s, "\n", sep="")
                          vall = c(vall, s)
                        }
                     }
                      return( list(stable=mean(vall), loss=1))
                   }, evalTrial=test_trials, test_run_freq=5
)
ro = run_options # for debug
id_patt = 1

#model_file = sprintf("%s/R/%s_%dx%d_lr%3.1f_lws_%3.1f", dir, data, M, N, run_options$learning_rate, run_options$learn_window_size)

model_file = sprintf("%s/%s_%dx%d", dir, data, M, N)
#model_file = "/home/alexeyche/prog/sim/test/synthetic_control_50x50_100"
if(runmode=="run") {
  if(file.exists(paste(model_file, ".idx", sep=""))) {  
    W = loadMatrix(model_file, 1)
    invisible(sapply(1:N, function(id) { 
      id_to_conn = which(W[,id] != 0)
      neurons$weights[[id]] = W[id_to_conn, id] 
      neurons$id_conns[[id]] = id_to_conn
    }))
    cat("Load - Ok\n")
  } else {
    cat("Can't find file for model ", model_file, "\n")
  }
}
patterns = gr1$patterns
layers = SimLayers( list(neurons) )
input_neurons = gr1

run_net(gr1, layers, run_options)

W = get_weights_matrix(list(neurons))
if(runmode == "learn") {
  saveMatrixList(model_file, list(W))
}
