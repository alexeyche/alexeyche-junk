

require(Rsnn)
source('ucr_ts.R')
source('../../scripts/eval_dist_matrix.R')

sample_size = 60

c(train, test) := read_ts_file(synth, sample_size, "~/prog/sim/ts")


fn_opt = function(input) {
    fb = RGammatoneFB$new()
    
    low_f = input[1]
    high_f = input[2]
    freqs = seq(low_f, high_f, length.out=100)
    sampling_rate = input[3]
    hrect = 1
    verbose = 0
    
    train_fb = list()
    for(x in train) {
        o = fb$calc(x$data, freqs, sampling_rate, hrect, verbose)
        mem = do.call(rbind, o$membrane)
        train_fb[[length(train_fb)+1]] = list(data=mem, label=x$label)
    }
    
    dm = matrix(0, length(train_fb), length(train_fb))
    for(i in 1:length(train_fb)) {
        for(j in 1:i) {
            d = sum((train_fb[[i]]$data - train_fb[[j]]$data)^2)
            dm[i,j] = d
            dm[j,i] = d
        }
    }
    data$distance_matrix = lapply(1:nrow(dm), function(i) dm[i,])
    data$labels = sapply(train_fb, function(x) x$label)
    return(calculate_criterion(data, FALSE))
}

o = optim(c(1, 200, 1000), fn_opt, lower=0.1, method="Brent")
#o = DEoptim(fn_opt, , c(25, 5000, 10000))


#fn_opt(c(0.1,200,575))