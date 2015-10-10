
require(Rdnn)
require(kernlab)

source(scripts.path("kernel_methods.R"))

setVerboseLevel(1)

run_neurons = function(
    ts
    , tau_m = 5.0
    , tau_ref = 2.0
    , thresh = 0.25
    , jobs = 2
) {
    M = nrow(ts$values)

    s = RSim$new()
    const = s$getConst()
    const$setElement(
        "LeakyIntegrateAndFire",
        list(tau_m=tau_m, rest_pot=0.0, tau_ref=tau_ref, noise=.0)
    )
    const$setElement("Determ", list(threshold=thresh))
    const$addLayer(
        list(size=M, neuron="LeakyIntegrateAndFire", act_function="Determ", input="InputTimeSeries")
    )

    s$build()
    s$setTimeSeries(ts, "InputTimeSeries")
    s$run(jobs)

    return(s$getSpikes())
}

run.svm = function(K, holdout, nu=0.5) {
    y = factor(colnames(K))
    trainK = as.kernelMatrix(K[-holdout,-holdout])
    model = ksvm(trainK, y=y[-holdout], kernel="matrix", type="nu-svc", nu=nu)
    testK = as.kernelMatrix(K[holdout, -holdout][,SVindex(model), drop=F])
    preds <- predict(model, testK)
    error_rate = 1 - sum(preds == y[holdout]) / length(holdout)
    cat("Error: ", error_rate, "\n")
    return(error_rate)
}

plot.kernel.pca = function(K) {
    K = as.kernelMatrix(K)
    kpc = kpca(K, features=2)
    y = factor(colnames(K))
    plot(rotated(kpc), col=y)
}

c(train_ts, test_ts) := prepare.ucr.data(300, UCR.SYNTH, gap_between_patterns = 60)

ts_whole = cat.ts(train_ts, test_ts)


M = 100
low_f = 10
high_f = 300
samp_rate = 1000
seq.fun = log.seq

data_conv = conv.gammatones(ts_whole, seq.fun(low_f, high_f, length.out=M), samp_rate)
spikes = run_neurons(data_conv, tau_m=5.0, tau_ref=2.0, thresh=0.2)

c(train_spikes, test_spikes) := split.spikes(spikes, tail(train_ts$ts_info$labels_timeline, n=1))

proto.write.spikes(spikes.path("ucr_train_spikes.pb"), train_spikes) 
proto.write.spikes(spikes.path("ucr_test_spikes.pb"), test_spikes) 

K = kernel.run(spikes, "Epsp(10)", "RbfDot(0.05)", jobs=8)

train_size = length(train_ts$ts_info$labels_timeline)
test_size = length(test_ts$ts_info$labels_timeline)

holdout=(train_size+1):(train_size+test_size)

plot.kernel.pca(K)
run.svm(K, holdout)

dnn.clean.heap()

labs = rownames(K)
sel = c("1", "2", "3", "4", "5","6")
idx = which(labs %in% sel)
Ksel = K[idx, idx]

c(y, M, N, A) := KFD(Ksel)

metrics_str = sprintf("%f, %f", tr(M)/tr(N), tr(A))

ans = Ksel %*% y[, 1:2]

par(mfrow=c(1,2))
plot(Re(ans[,1]), col=as.integer(rownames(Ksel)), main=metrics_str) 
plot(Re(ans), col=as.integer(rownames(Ksel)))



