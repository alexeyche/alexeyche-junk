#setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
setwd("~/prog/alexeyche-junk/cns/R/srm/new")
require(snn)
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


source('include.R')
source('ucr_ts.R')
source('eval_funcs.R')
source('layers.R')

#dir = "/home/alexeyche/my/sim"
dir = "~/prog/sim"
#system(sprintf("find %s -name \"*.png\" -type f -exec rm -f {} \\;", dir))
ID_MAX=0

M = 50
N = 10

data = synth # synthetic control

set.seed(1234)
c(train_dataset, test_dataset) := read_ts_file(data, '~/prog/sim')
elems = 50
train_dataset = train_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150,elems),
                               sample(151:200, elems), sample(201:250,elems), sample(251:300,elems))] # cut
test_dataset = test_dataset[c(sample(1:50, elems), sample(51:100, elems), sample(101:150, elems),
                               sample(151:200, elems), sample(201:250,elems), sample(251:300, elems))]
ans = ucr_test(train_dataset, test_dataset, eucl_dist_alg)

#train_dataset = train_dataset[c(1,101, 2, 102, 3, 103, 4, 104, 5, 105)] # cut


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
#model_file = '/home/alexeyche/prog/sim/verylast_run/synthetic_control_50x10_9'
#model_file = '/home/alexeyche/prog/sim/verylast_run3/synthetic_control_50x10_27'
#model_file = '/home/alexeyche/prog/sim/verylast_run4/synthetic_control_50x10_10'  # quazi_good
#model_file = '/home/alexeyche/prog/sim/spear_run/run20058/synthetic_control_50x10_22'
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

trials = 10

run_options = list(T0 = 0, Tmax = duration, dt = 0.5, learning_rate = 0.5,
                   learn_window_size = 300, mode="run", collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_2spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1, evalTrial = trials
)


net_neurons = SimLayers(list(neurons))

o_train = evalNet(gr1$patterns, run_options, constants, net_neurons$l)
o_test = evalNet(gr2$patterns, run_options, constants, net_neurons$l)

#window=10
for(window in c(8,10, 12, 14, 16, 20)) {
  for(sigma in c(2,3,4, 6, 8, 10)) {
    k_out_train = lapply(o_train$spikes, function(st) kernelPass_spikes(st, list(sigma = sigma, window = window, T0 = 0, Tmax = duration, quad = 256)) )
    k_out_test = lapply(o_test$spikes, function(st) kernelPass_spikes(st, list(sigma = sigma, window = window, T0 = 0, Tmax = duration, quad = 256)) )
     
    k_out_train_m = mean_on_trials(k_out_train, trials)
    k_out_test_m = mean_on_trials(k_out_test, trials)
    
    cat("k: ", sigma, "w: ", window," ")
    perf = ucr_test(k_out_train_m, k_out_test_m, eucl_dist_alg)
    
  }
}

for(binSize in c(10, 12.5, duration/7)) {
  k_out_train_m = post_process_set(o_train$spikes, trials, 0, duration, binKernel, binSize)
  k_out_test_m = post_process_set(o_test$spikes, trials, 0, duration, binKernel, binSize)
  perf = ucr_test(k_out_train_m, k_out_test_m, eucl_dist_alg)
}

#sigma = 10
#window = 10


#
#for(bw in seq(50, 150, by=10)) {
#  sm_st_tr = smooth_stat(o_train$stat, bw)
#  sm_st_test = smooth_stat(o_test$stat, bw)
#  sm_st_tr_m = mean_on_trials(sm_st_tr, trials)
#  sm_st_test_m = mean_on_trials(sm_st_test, trials)
#  cat("bw:", bw, " ")
#  perfs = ucr_test(sm_st_tr_m, sm_st_test_m, eucl_dist_alg)
#}


#R> sp_proc = post_process_set2(net_all_sp, 15, 0, 300, 10, 5)
#R> perf = ucr_test(sp_proc[1:300], sp_proc[301:600], eucl_dist_alg)
#The error rate is  0.1033333 
# R> model_file
# [1] "/home/alexeyche/prog/sim/last_last_1/R/synthetic_control_50x10_30"



# R> confm_base
# [,1] [,2] [,3] [,4] [,5] [,6]
# [1,]   22   10    3    4    5    6
# [2,]    0   50    0    0    0    0
# [3,]    0    0   49    0    1    0
# [4,]    0    0    0   50    0    0
# [5,]    0    0    3    0   47    0
# [6,]    0    0    0    4    0   46
# R> confm
# [,1] [,2] [,3] [,4] [,5] [,6]
# [1,]   45    5    0    0    0    0
# [2,]    1   48    0    1    0    0
# [3,]    1    0   47    0    2    0
# [4,]    0    0    0   48    0    2
# [5,]    0    0    7    0   43    0
# [6,]    2    0    0   10    0   38
