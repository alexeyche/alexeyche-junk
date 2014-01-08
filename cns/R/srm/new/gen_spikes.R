#!/usr/bin/RScript

TSNeurons <- setRefClass("TSNeurons", fields = list(M = "vector", patterns = "list"), 
                                    methods = list(
                                    loadPattern = function(file, pattDur, class) {
                                        l <- length(patterns)
                                        rawdata <- read.table(file, sep=",")
                                        patt <- list(file=file, pattDur=pattDur, dt=pattDur/nrow(rawdata), rawdata=rawdata[,1], len=nrow(rawdata), class=class)
                                        data <- matrix(-Inf, nrow=M, ncol = patt$len)
                                        hb <- max(rawdata)
                                        lb <- min(rawdata)
                                        dt <- (hb-lb)/(M-1)
                                        patt_dt <- 0
                                        for(ri in 1:patt$len) {
                                            val <- rawdata[ri,1]
                                            ind_n <- floor((val-lb)/dt)+1
                                    #       cat("ind_n: ", ind_n, " val:    val, "\n")
                                            patt_dt <- patt_dt + pattDur/patt$len
                                            data[ind_n, ri] <- patt_dt
                                        }
                                        patt$data <- spikeMatToSpikeList(data)
                                        patterns[[l+1]] <<- patt                                  
                                    },
                                    preCalculate = function(T0, Tmax, dt) {
                                        T0_current <- T0
                                        for(pattern in patterns) {
                                            patt_ti <- 0
                                            for(t in seq(T0_current, Tmax, by=dt)) {
                                                patt_index <- patt_ti/pattern$dt
                                                cat("t: ", t, " patt_index: ", patt_index, " patt_ti: ", patt_ti, "\n") 
                                                patt_ti <- patt_ti + dt    
                                                if(patt_index > pattern$len) {
                                                    T0_current = t
                                                    break
                                                }

                                            }
                                            if(T0_current>=Tmax) {
                                                break
                                            }
                                        }
                                    }))



#file <- "/home/alexeyche/prog/sim/stimuli/d1.csv"

plot_patt <- function(patt) {
    pl_mat <- NULL
    for(i in 1:ncol(patt$data)) {
        n_fired <- which(patt$data[,i] >= 0)
        x <- patt$data[n_fired,i]
        y <- n_fired
        pl_mat <- rbind(pl_mat, c(x, y))
    }
    plot(pl_mat)
}
#plot_patt(gr1$patterns[[1]])
#preCalculate(gr1$patterns[[1]], 0, 300, 0.5)
spikeMatToSpikeList <- function(m) {
    l <- list()
    for(ri in 1:nrow(m)) {
        l[[ri]] <- m[ri, m[ri, ]>=0]
    }
    return(l)
}


