setwd("~/my/git/alexeyche-junk/cns/R/srm/new")
#setwd("~/prog/alexeyche-junk/cns/R/srm/new")
source('include.R')
source('ucr_ts.R')

dir = "/home/alexeyche/my/sim"
#dir = "~/prog/sim"
#system(sprintf("find %s -name \"*.png\" -type f -exec rm -f {} \\;", dir))
ID_MAX=0

M = 50
N = 10

data = synth # synthetic control

c(train_dataset, test_dataset) := read_ts_file(data)

train_dataset = train_dataset[c(1,101, 2, 102, 3, 103, 4, 104, 5, 105)] # cut

duration = 100

N = 10
start_w.M = 1 #matrix(rnorm( M*N, mean=2, sd=0.5), ncol=N, nrow=M)
start_w.N = 1 #matrix(rnorm( (N-1)*N, mean=2, sd=0.5), ncol=N, nrow=(N-1))
M = 50
dt = 0.5

gr1 = TSNeurons(M = M)
neurons = SRMLayer(N, start_w.N)

gr1$loadPatterns(train_dataset, duration, dt, lambda=8)
patt_len = length(gr1$patterns)
neurons$connectFF(gr1$ids, start_w.M)

start_w = 5


model = "===="

model_file = sprintf("%s/R/%s", dir, model)

if(file.exists(paste(model_file, ".idx", sep=""))) {
    W = loadMatrix(model_file, 1)
    invisible(sapply(1:N, function(id) { 
      layers[[1]]$weights[[id]] = W[1:length(layers[[1]]$id_conns[[id]]),id] 
    } 
    ))
} else {
  cat("Can't find file for model ", model_file, "\n")
}



run_options = list(T0 = 0, Tmax = duration, dt = 0.5, learning_rate = 0.5,
                   learn_window_size = 100, mode="run", collect_stat=TRUE, 
                   target_set = list(target_function_gen = random_2spikes_tf, depress_null=FALSE),
                   learn_layer_id = 1
)
patterns = gr1$patterns
trials = 1
net_all = list()
u_all = list()
p_all = list()
net_neurons = list(neurons)
for(id_patt in 1:length(patterns)) {
  for(trial in 1:trials) {
    net = list()
    net[gr1$ids] = patterns[[id_patt]]$data
    net[neurons$ids] = -Inf
    run_options$label = patterns[[id_patt]]$label
    
    c(net, net_neurons, stat, mean_grad) := run_srm(net_neurons, net, run_options)
    net = lapply(net[neurons$ids], function(sp) sp[sp != -Inf])
    glob_id = trial+(id_patt-1)*trials
    net_all[[glob_id]] = list(data=net, label=patterns[[id_patt]]$label)
    u_all[[glob_id]] = list(data=stat[[1]]$u, label=patterns[[id_patt]]$label)
    p_all[[glob_id]] = list(data=stat[[1]]$p, label=patterns[[id_patt]]$label)
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
