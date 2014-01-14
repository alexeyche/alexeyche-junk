setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
source('include.R')

dir = "/home/alexeyche/my/sim/R"
#system(sprintf("find %s -name \"*.png\" -type f -exec rm -f {} \\;", dir))

M = 50
N = 10

gr1 = TSNeurons(M = M, patterns = list())

#file <- "/home/alexeyche/prog/sim/stimuli/sd1.csv"
#file2 <- "/home/alexeyche/prog/sim/stimuli/sd2.csv"
file <- "/home/alexeyche/my/sim/stimuli/sd1.csv"
file2 <- "/home/alexeyche/my/sim/stimuli/sd2.csv"

gr1$loadPattern(file, 100, 1)
gr1$loadPattern(file2, 100, 2)
id_m = 1:M
id_n = (M+1):(M+N)

start_w = 5
weights = list()
id_conns = list()
ids = NULL
for(i in 1:N) {
  conn <- id_m
  conn <- c(conn, id_n[id_n != id_n[i]]) # id of srm neurons: no self connections
  w <- c(rep(start_w, M), rep(start_w/4, N-1))
  id_conns[[i]] = conn
  weights[[i]] = w
  ids = c(ids, id_n[i])
}

neurons = SRMLayer(id_conns = id_conns, weights = weights, ids=ids) 
layers = list(neurons)
model = "50x10_lr0.5_lws_100.0"

model_file = sprintf("%s/%s", dir, model)

if(file.exists(paste(model_file, ".idx", sep=""))) {
    W = loadMatrix(model_file, 1)
    invisible(sapply(1:N, function(id) { 
      layers[[1]]$weights[[id]] = W[1:length(layers[[1]]$id_conns[[id]]),id] 
    } 
    ))
} else {
  cat("Can't find file for model ", model_file, "\n")
}

null_pattern.N = list()
for(i in 1:N) {
  null_pattern.N[[i]] <- -Inf
}


run_options = list(T0 = 0, Tmax = 100, dt = 0.5, learning_rate = 0.5,
                   learn_window_size = 100, mode="run", collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_2spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1
)
patterns = gr1$patterns
trials = 30
net_all = list()
u_all = list()
p_all = list()
for(id_patt in 1:length(patterns)) {
  for(trial in 1:trials) {
    net = list()
    net[id_m] = patterns[[id_patt]]$data
    net[id_n] = null_pattern.N
    run_options$class = patterns[[id_patt]]$class
    
    c(net, layers, stat, mean_grad) := run_srm(layers, net, run_options)
    net = lapply(net[id_n], function(sp) sp[sp != -Inf])
    glob_id = trial+(id_patt-1)*trials
    net_all[[glob_id]] = list(data=net, label=patterns[[id_patt]]$class)
    u_all[[glob_id]] = list(data=stat[[1]]$u, label=patterns[[id_patt]]$class)
    p_all[[glob_id]] = list(data=stat[[1]]$p, label=patterns[[id_patt]]$class)
  }
}

split_data <- function(data, ratio=0.15) {
  N = length(data)
  ind = sample(N)
  spl_i = ratio*N
  test_i = ind[1:spl_i]
  train_i = ind[(spl_i+1):N]
  return(list(data[train_i], data[test_i]))
}

c(train, test) := split_data(u_all, ratio=0.1)
c(train, test) := split_data(net_all, ratio=0.1)

binKernel <- function(net_data, T0, Tmax, binSize=10) {
  maxl = max(sapply(net_data, length))
  breaks = seq(T0, Tmax, by=10)
  lb = length(breaks)
  for(i in 1:maxl) {
    it_sp = sapply(net_data, function(sp) if(length(sp)>=i) sp[i] else NA)
  }
}
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
