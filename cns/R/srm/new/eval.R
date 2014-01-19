#setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
setwd("~/prog/alexeyche-junk/cns/R/srm/new")
source('include.R')
source('ucr_ts.R')

#dir = "/home/alexeyche/my/sim/R"
dir = "~/prog/sim"
#system(sprintf("find %s -name \"*.png\" -type f -exec rm -f {} \\;", dir))
ID_MAX=0

M = 50
N = 10

data = synth # synthetic control

c(train_dataset, test_dataset) := read_ts_file(data)

#train_dataset = train_dataset[c(1,101, 2, 102, 3, 103, 4, 104, 5, 105)] # cut

duration = 100

N = 10
start_w.M = 4 #matrix(rnorm( M*N, mean=2, sd=0.5), ncol=N, nrow=M)
start_w.N = 3 #matrix(rnorm( (N-1)*N, mean=2, sd=0.5), ncol=N, nrow=(N-1))
M = 50
dt = 0.5

gr1 = TSNeurons(M = M)
neurons = SRMLayer(N, start_w.N)

gr1$loadPatterns(train_dataset, duration, dt, lambda=8)
gr1$loadPatterns(test_dataset, duration, dt, lambda=8)
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




model_file = sprintf("%s/R/%s_%dx%d", dir, data, M, N)


if(file.exists(paste(model_file, ".idx", sep=""))) {
    W = loadMatrix(model_file, 1)
    
    invisible(sapply(1:N, function(id) { 
      id_to_conn = which(W[,id] != 0)
      neurons$weights[[id]] = W[id_to_conn, id] 
      neurons$id_conns[[id]] = id_to_conn
    }))
} else {
  cat("Can't find file for model ", model_file, "\n")
}

net_neurons = list(neurons)

run_options = list(T0 = 0, Tmax = duration, dt = 0.5, learning_rate = 0.5,
                   learn_window_size = 100, mode="run", collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_2spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1
)
#patterns = gr1$patterns[1:length(train_dataset)] #[c(1:10,51:60, 101:110, 151:160, 201:210, 251:260)]
patterns = gr1$patterns #[(length(train_dataset)+1):(length(train_dataset)+length(test_dataset))] #[c(1:10,51:60, 101:110, 151:160, 201:210, 251:260)]

trials = 6
net_all = list()
u_all = list()
p_all = list()

for(id_patt in 1:length(patterns)) {
  for(trial in 1:trials) {
    net = list()
    net[gr1$ids] = patterns[[id_patt]]$data
    net[neurons$ids] = -Inf
    run_options$class = patterns[[id_patt]]$class
    
    c(net, net_neurons, stat, mean_grad) := run_srm(net_neurons, net, run_options)
    net = lapply(net[neurons$ids], function(sp) sp[sp != -Inf])
    glob_id = trial+(id_patt-1)*trials
    net_all[[glob_id]] = list(data=net, label=patterns[[id_patt]]$label)
    u_all[[glob_id]] = list(data=stat[[1]]$u, label=patterns[[id_patt]]$label)
    p_all[[glob_id]] = list(data=stat[[1]]$p, label=patterns[[id_patt]]$label)    
  }
}

split_data <- function(data, ratio=0.3) {
  N = length(data)
  ind = sample(N)
  spl_i = ratio*N
  test_i = ind[1:spl_i]
  train_i = ind[(spl_i+1):N]
  return(list(data[train_i], data[test_i]))
}

#c(train, test) := split_data(u_all, ratio=0.1)
#c(train, test) := split_data(net_all, ratio=0.5)
#train = lapply(train, function(d) { d$data = binKernel(d$data, 0, 100, 10); return(d) })
#test = lapply(test, function(d) { d$data = binKernel(d$data, 0, 100, 10); return(d) })


#ucr_test(train, test, eucl_dist_alg)

binKernel <- function(net_data, T0, Tmax, binSize=10) {
  maxl = max(sapply(net_data, length))
  breaks = seq(T0, Tmax, by=binSize)
  lb = length(breaks)-1
  hist_ans = matrix(0, length(net_data), lb)
  
  for(i in 1:maxl) {
    it_sp = sapply(net_data, function(sp) if(length(sp)>=i) sp[i] else -1)
    time_ids = ceiling((it_sp-T0)/binSize)
    for(j in 1:length(time_ids)) {
      if(time_ids[j]>0) {
        hist_ans[j,  time_ids[j]] = hist_ans[j,  time_ids[j]] + 1
      }
    } 
  }
  return(hist_ans)
}
#  binKernel(net_all[[2]]$data, 0, 100, 5)
kernSize=10
spikes_proc = list()
for(id_patt in 1:length(patterns)) {
  st_i = (id_patt-1)*trials+1
  fin_i = st_i + trials -1
  data_proc_l = lapply(net_all[st_i:fin_i], function(n) binKernel(n$data, 0, duration, kernSize))
  data_proc = array(0, dim=c(N, duration/kernSize, length(data_proc_l)))
  for(i in 1:length(data_proc_l)) {
    data_proc[,,i] = data_proc_l[[i]]
  }
  spikes_proc[[id_patt]] = list(data=apply(data_proc, c(1,2), mean), label=patterns[[id_patt]]$label)
}
ucr_test(spikes_proc[1:300], spikes_proc[301:600], eucl_dist_alg)
#dist = matrix(0, nrow=length(spikes_proc), ncol=length(spikes_proc))
#for(i in 1:length(spikes_proc)) {
#  for(j in 1:length(spikes_proc)) {
#    dist[i,j] =  sum((spikes_proc[[i]]$data - spikes_proc[[j]]$data)^2)
#  }
#}


# val "pot"
#source_data = "pot" # prob, spike
#source_data = "prob"

#m = nrow(p_all)
#if(source_data == "prob") {  
#  pw = p_all[sample(m),]
#  #x = 1-exp(-pw[,1:10])
#  x = pw[,1:10]
#  y = pw[,11]
#} else 
#  if(source_data == "pot") {
#    uw = u_all[sample(m),]
#    y = uw[,11]    
#    x = normalizeData(uw[,1:10], "0_1")
#}

#y <- decodeClassLabels(y)
#set <- splitForTrainingAndTest(x, y, ratio=0.15)


#mod = rbfDDA(set$inputsTrain, set$targetsTrain)
#mod_rbf = rbf(set$inputsTrain, set$targetsTrain, size=10, maxit=100, initFuncParams=c(-4,4, 0, 0.02, 0.04))
#mod = mlp(set$inputsTrain, set$targetsTrain, size=5, maxit=500)
#plotIterativeError(mod)
#filled.contour(weightMatrix(mod))
#predictions = predict(mod, set$inputsTest)
#plotRegressionError(predictions[,2], set$targetsTest[,2])
#confusionMatrix(set$targetsTrain,fitted.values(mod))
#confusionMatrix(set$targetsTest,predictions)

#plotROC(fitted.values(mod)[,2], set$targetsTrain[,2])
#plotROC(predictions[,2], set$targetsTest[,2])
