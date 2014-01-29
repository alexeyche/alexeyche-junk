setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
#setwd("~/prog/alexeyche-junk/cns/R/srm/new")
require(snnSRM)
source('include.R')
source('ucr_ts.R')
source('eval_funcs.R')
source('layers.R')

dir = "/home/alexeyche/my/sim"
#dir = "~/prog/sim"
#system(sprintf("find %s -name \"*.png\" -type f -exec rm -f {} \\;", dir))
ID_MAX=0

M = 50
N = 10

data = synth # synthetic control

set.seed(1234)
c(train_dataset, test_dataset) := read_ts_file(data)
#train_dataset = train_dataset[c(sample(1:50, 10), sample(51:100, 10), sample(101:150,10),
#                                  sample(151:200, 10), sample(201:250,10), sample(251:300,10))] # cut
#test_dataset = test_dataset[c(sample(1:50, 10), sample(51:100, 10), sample(101:150, 10),
#                                sample(151:200, 10), sample(201:250,5), sample(251:300, 10))]
ans = ucr_test(train_dataset, test_dataset, eucl_dist_alg)

#train_dataset = train_dataset[c(1,101, 2, 102, 3, 103, 4, 104, 5, 105)] # cut

duration = 300

N = 10
start_w.M = 4 #matrix(rnorm( M*N, mean=2, sd=0.5), ncol=N, nrow=M)
start_w.N = 3 #matrix(rnorm( (N-1)*N, mean=2, sd=0.5), ncol=N, nrow=(N-1))
M = 50
dt = 0.5

gr1 = TSNeurons(M = M)
gr2 = TSNeurons(M = M, ids_c = 1000:(1000+M))
neurons = SRMLayer(N, start_w.N)

gr1$loadPatterns(train_dataset, duration, dt, lambda=5)
gr2$loadPatterns(test_dataset, duration, dt, lambda=5)
patt_len = length(gr1$patterns)


connection = matrix(gr1$ids, nrow=length(gr1$ids), ncol=N)
connect_window = N*2
step = M/N
overlap = 1
for(ni in 0:(N-1)) {
  if(ni != 0) connection[1:((ni*step)-overlap),ni+1] = 0    
  if(ni != N-1) connection[((ni*step)+step+1+overlap):M,ni+1] = 0
}

neurons$connectFF(connection, start_w.M, 1:N )


args <- commandArgs(trailingOnly = TRUE)
if(length(args)>0) {
  model_file = args
} else {
  model_file = sprintf("%s/R/%s_%dx%d", dir, data, M, N)
}
model_file=""

if(file.exists(paste(model_file, ".idx", sep=""))) {
    W = loadMatrix(model_file, 1)
    
    invisible(sapply(1:N, function(id) { 
      id_to_conn = which(W[,id] != 0)
      neurons$weights[[id]] = W[id_to_conn, id] 
      neurons$id_conns[[id]] = id_to_conn
    }))
  paste("model",model_file, "successfully loaded\n")
} else {
  cat("Can't find file for model ", model_file, "\n")
}



#patterns = gr1$patterns[1:length(train_dataset)] #[c(1:10,51:60, 101:110, 151:160, 201:210, 251:260)]
patterns = gr1$patterns #[(length(train_dataset)+1):(length(train_dataset)+length(test_dataset))] #[c(1:10,51:60, 101:110, 151:160, 201:210, 251:260)]


trials = 3

run_options = list(T0 = 0, Tmax = duration, dt = 0.5, learning_rate = 0.5,
                   learn_window_size = 300, mode="run", collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_2spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1, evalTrial = trials
)

net_neurons = SimLayers(list(neurons))


o_train = evalNet(gr1$patterns, run_options, constants, net_neurons$l)
o_test = evalNet(gr2$patterns, run_options, constants, net_neurons$l)


sigma = 10
window = 10

k_out_train = lapply(o_train$spikes, function(st) kernelPass_spikes(st, list(sigma = sigma, window = window, T0 = 0, Tmax = duration, quad = 256)) )
k_out_test = lapply(o_test$spikes, function(st) kernelPass_spikes(st, list(sigma = sigma, window = window, T0 = 0, Tmax = duration, quad = 256)) )

k_out_train_m = mean_on_trials(k_out_train, trials)
k_out_test_m = mean_on_trials(k_out_test, trials)

perf = ucr_test(k_out_train_m, k_out_test_m, eucl_dist_alg)

for(bw in seq(50, 150, by=10)) {
  sm_st_tr = smooth_stat(o_train$stat, bw)
  sm_st_test = smooth_stat(o_test$stat, bw)
  sm_st_tr_m = mean_on_trials(sm_st_tr, trials)
  sm_st_test_m = mean_on_trials(sm_st_test, trials)
  cat("bw:", bw, " ")
  perfs = ucr_test(sm_st_tr_m, sm_st_test_m, eucl_dist_alg)
}
  



