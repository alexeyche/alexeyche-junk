


plotl <- function(x) {
    plot(x, type="l")
}

':=' = function(lhs, rhs) {
    if(is.list(rhs)) {
        frame = parent.frame()
        lhs = as.list(substitute(lhs))
        if (length(lhs) > 1)
        lhs = lhs[-1]
        if (length(lhs) == 1) {
        do.call(`=`, list(lhs[[1]], rhs), envir=frame)
        return(invisible(NULL)) }
        if (is.function(rhs) || is(rhs, 'formula'))
        rhs = list(rhs)
        if (length(lhs) > length(rhs))
        rhs = c(rhs, rep(list(NULL), length(lhs) - length(rhs)))
        for (i in 1:length(lhs))
        do.call(`=`, list(lhs[[i]], rhs[[i]]), envir=frame)
        return(invisible(NULL))
    }
    if(is.vector(rhs)) {
        mapply(assign, as.character(substitute(lhs)[-1]), rhs,
        MoreArgs = list(envir = parent.frame()))
        invisible()
    }
}


require(lattice, quietly=TRUE)

prast_mpl = function(spikes,T0=0, Tmax=Inf) {
    x = c()
    y = c()
    cex = c()
    tv = spikes$t
    sv = spikes$s
    fiv = spikes$fi

    for(i in 1:length(tv)) {
        if((tv[i]<T0)||(tv[i])>Tmax) next
        x = c(x, tv[i])
        y = c(y, fiv[i])
        cex = c(cex, sv[i])

    }
    pl_size = Sys.getenv("MPLMATCH_PLOT_SIZE")
    pl_size = as.numeric(pl_size)
    if(is.na(pl_size)) {
        pl_size = 1.0
    }

    xyplot(y ~ x, list(x = x, y = y), xlim=c(T0, max(x)), cex=cex*pl_size,  col = "black")
}

plot_rastl <- function(spikes_list, i=NULL, nids=NULL, T0=0, Tmax=Inf, xlim=NULL, ...) {
    if(!is.null(xlim)) {
        T0 = xlim[1]
        Tmax = xlim[2]
    }
    if(!is.null(i)) {
        T0 = min(sapply(spikes_list$info[i], function(i) i$start_time))
        Tmax = max(sapply(spikes_list$info[i], function(i) i$start_time + i$duration))
    }
    raster = spikes_list$values
    
    x <- c()
    y <- c()
    if(is.null(nids)) {
        nids = 1:length(raster)    
    }
    for(ni in nids) {
        rast = raster[[ni]]
        rast = rast[rast >= T0]
        rast = rast[rast < Tmax]
        x <- c(x, rast)
        y <- c(y, rep(ni, length(rast)))
    }
    if(length(x) == 0) {
      stop("Got empty raster plot")
    }
    if(Tmax == Inf) {
        Tmax = max(x)
    }
    return(xyplot(y~x,list(x=x, y=y), xlim=c(T0, Tmax), col="black", ...))
}

prast = plot_rastl

plot.SpikesList = function(obj, ...) { 
    prast(obj, ...) 
}

gr_pl = function(m, ...) {
    levelplot(m, col.regions=colorRampPalette(c("black", "white")), ...)
}


plot.TimeSeries = function(obj, i=NULL, ...) {
    if(is.null(nrow(obj$values))) {
        plot(obj$values, type="l", ...)
    } else
    if(nrow(obj$values) == 1) {
        plot(obj$values[1, ], type="l", ...)
    } else {
        gr_pl(t(obj$values), ...)
    }
}


measureSpikeCor = function(net, dt) {
    N = length(net)
    Tmax = max(sapply(net, function(x) if(length(x)>0) max(x) else -Inf))

    net_m = matrix(0, nrow=N, ncol=Tmax/dt)
    for(ni in 1:N) {
        net_m[ni, ceiling(net[[ni]]/dt) ] <- 1
    }
    cor_m = matrix(0, nrow=N, ncol=N)
    for(ni in 1:100) {
        for(nj in 1:100) {
            if((all(net_m[ni,] == 0))||(all(net_m[nj,] == 0))||(ni == nj)) {
                cor_m[ni, nj] = 0
            } else {
                cor_m[ni, nj] = cor(net_m[ni, ], net_m[nj, ])
            }
        }
    }

    return(cor_m)
}

readConst = function(const) {
    const_cont = scan(const,what=character(), sep="\n")
    const_cont = gsub("(//|#).*","", const_cont)
    const_cont = paste(const_cont, sep="\n", collapse="")
    return(const_cont)
}

blank_net = function(N) {
    if(N<=0) return(list())
    net = list()
    for(i in 1:N) {
        net[[i]] = numeric(0)
    }
    return(net)
}

safe.log = function(x) {
    if(x == 0) return(0)
    return(log(x))
}

log.seq = function(from, to, length.out) {
    return(exp(seq(safe.log(from), safe.log(to), length.out=length.out)))
}



require(zoo, quietly=TRUE, warn.conflicts=FALSE)

interpolate_ts = function(ts, interpolate_size) {
    out_approx = NA
    
    while(length(out_approx) != interpolate_size) {
        out = rep(NA, interpolate_size)
        iter <- 0
        for(i in 1:length(ts)) {
            iter = iter+length(out)/length(ts)
            ct = floor(signif(iter, digits=5))                                        
            out[ct] = ts[i]
        }
        out_approx = na.approx(out)
        ts = out_approx
    }
    
    return(out_approx)
}

cats = function(s, ...) {
    sf = sprintf(s, ...)
    cat(sf)
}


parse.options = function(args, opts) {
    usage = function() {
        cat("Available options: \n")
        for(opt in names(opts)) {
            o = opts[[opt]]
            if("default" %in% names(o)) {
                cats("\t%15.15s    %s, default: %s\n", opt, o$description, o$default)    
            } else {
                cats("\t%15.15s    %s\n", opt, o$description)
            }
        }
        cats("\t%15.15s    for this message\n", "--help")
        stop()
    }
    if((length(args) == 5)||(length(grep("--help", args))>0)) {
        usage()
    }
    for(opt in names(opts)) {
        o = opts[[opt]]
        idx = grep(opt, args)
        if(length(idx)>0) {                        
            if(length(args)<(idx+1)) {
                cats("Can't find argument for %s\n", opt)
                usage()
            }
            v = args[idx+1]
            if("process" %in% names(o)) {
                v = o$process(v)
            }
            opts[[opt]]$value = v
        } else {
            if("default" %in% names(o)) {
                opts[[opt]]$value = o$default    
            } else {
                cats("Can't find value for %s. It hasn't any defaults", opt)
                usage()                
            }        
        }
    }
    return(lapply(opts, function(x) x$value))
}


