

require(Rsnn)
source('ucr_ts.R')
source('../../scripts/eval_dist_matrix.R')

sample_size = 60

c(train, test) := read_ts_file(synth, sample_size, "~/prog/sim/ts")

filter_with_gammatones = function(set, freqs, sampling_rate, hrect, verbose) {
    fb = RGammatoneFB$new()
    oset = list()
    for(x in set) {
        o = fb$calc(x$data, freqs, sampling_rate, hrect, verbose)
        mem = do.call(rbind, o$membrane)
        oset[[length(oset)+1]] = list(data=mem, label=x$label)
    }
    return(oset)
}

clust_eval_set = function(set, pl=FALSE) {
    dm = matrix(0, length(set), length(set))
    for(i in 1:length(set)) {
        for(j in 1:i) {
            d = sum((set[[i]]$data - set[[j]]$data)^2)
            dm[i,j] = d
            dm[j,i] = d
        }
    }
    data$distance_matrix = lapply(1:nrow(dm), function(i) dm[i,])
    data$labels = sapply(set, function(x) x$label)
    return(calculate_criterion(data, pl))
}

lseq = function(from, to, length) {
    stopifnot(from>0)
    exp(seq(log(from), log(to), length.out = length))
}
scale = function(x, min, max, a, b) {
    ((b-a)*(x-min)/(max-min)) + a
}


fn_opt = function(input) {    
    low_f = input[1]
    high_f = input[2]
    sampling_rate = input[3]
    #low_f = scale(input[1],  cma_bounds[1], cma_bounds[2], 0.1, 200)
    #high_f = scale(input[2], cma_bounds[1], cma_bounds[2], 10, 2000)
    #sampling_rate = scale(input[3], cma_bounds[1], cma_bounds[2], 10, 2000)
    
    freqs = lseq(low_f, high_f, 100)
    
    cat(low_f, high_f, sampling_rate, "=")
    hrect = 1
    verbose = 0
    train_fb = filter_with_gammatones(train, freqs, sampling_rate, hrect, verbose)
    ev = clust_eval_set(train_fb)
    cat(ev,"\n")
    return(ev)
}


o = optim(c(1, 200, 1000), fn_opt, lower=0.1)

#o = DEoptim(fn_opt, , c(25, 5000, 10000))

