
add.to.list = function(l, v) {
    l[[length(l)+1]] = v
    return(l)
}

time.series = function(values, info=NULL) {
    o = list(values = values)
    if(is.null(info)) {
        info = list(ts.info(label="unknown_label", duration=length(values), start_time=0))
    }
    o$info = info
    class(o) <- "TimeSeries"
    return(o)
}

spikes.list = function(values, info=NULL) {
    if(is.null(info)) {
        info = list()
    }
    o = list(values = values, info = ts.info(info))
    class(o) <- "SpikesList"
    return(o)
}

ts.info = function(...) {
    ts_info = list(...)
    check.ts.info = function(inf) {
        if(! ("duration" %in% names(inf))) {
            stop("Need duration in time series sample specification")
        }
        if(! ("label" %in% names(inf))) {
            stop("Need label in time series sample specification")
        }
        if(! ("start_time" %in% names(inf))) {
            stop("Need start_time in time series sample specification")
        }
        if(!is.character(inf$label)) {
            stop("Label must be a string in time series specification")
        }            
    }
    if(length(ts_info) == 0) {
        return(list())
    }
    if((length(ts_info) == 1)&&(length(ts_info[[1]]) == 0)) {
        return(list())
    }
    if("duration" %in% names(ts_info)) {
        check.ts.info(ts_info)
    } else {
        for(o in ts_info) {
            check.ts.info(o)
        }
    }
    return(ts_info)
}

empty.ts = function() {
    time.series(values=c(), info=list())
}

empty.spikes = function(N=NULL) {
    l = list()
    if(!is.null(N)) {
        l = lapply(1:N, function(i) numeric(0))
    }
    spikes.list(values=l, info=NULL)
}

add.to.ts = function(ts, new_ts) {
    last_time = length(ts$values)
    ts$values = c(ts$values, new_ts$values)
    for(inf in new_ts$info) {
        ts$info = add.to.list(
            ts$info
          , list(label=info$label, duration=info$duration, start_time = last_time + info$start_time)
        )
    }
    return(ts)
}

add.to.spikes = function(sp, new_sp, gap=0, from=NULL) {
    if(length(sp$values) != length(new_sp$values)) {
        stop("Trying to add spikes with another size of population")
    }
    if(is.null(from)) {
        from = spikes.list.max.t(sp)    
    }    
    sp$values = lapply(
        1:length(sp$values)
      , function(ni) {
          c(sp$values[[ni]],  new_sp$values[[ni]] + from + gap)
      }
    )
    for(inf in new_sp$info) {
        sp$info = add.to.list(
            sp$info
          , list(label=inf$label, duration=inf$duration, start_time = from + inf$start_time + gap)
        )
    }
    return(sp)
}

cat.ts = function(...) {
    ts = list(...)
    fts = empty.ts()
    for(t in ts) {
        if(class(t) != "TimeSeries") stop("Expecting TimeSeries as input")
        fts = add.to.ts(fts, t)
    }
    return(fts)
}

cat.spikes = function(...) {
    spikes = list(...)
    
    if(class(spikes[[1]]) != "SpikesList") stop("Expecting SpikesList as input")
    fsp = empty.spikes(length(spikes[[1]]$values))
    
    for(s in spikes) {
        fsp = add.to.spikes(fsp, s)
    }
    return(fsp)
}

split.spikes = function(sp, number_to_split) {
    time_to_split = sp$info[[number_to_split]]$start_time
    left_idx = which(sp$ts_info$labels_timeline <= time_to_split)
    left_sp = empty.spikes()    
    right_sp = empty.spikes()
        
    left_sp$values = sapply(sp$values, function(spike_times) spike_times[ which(spike_times<=time_to_split) ] )
    right_sp$values = sapply(sp$values, function(spike_times) spike_times[ which(spike_times>time_to_split) ] - time_to_split)
    
    left_sp$info = sp$info[1:(number_to_split-1)]
    right_sp$info = sp$info[number_to_split:length(sp$info)]
    
    right_sp$info = lapply(right_sp$info, function(x) { x$start_time = x$start_time -time_to_split; return(x) })
    return(list(left_sp, right_sp))
}

spikes.list.max.t = function(sp) {
    max_t = max(sapply(sp$values, function(l) {
        if(length(l) == 0) {
            return(0)
        } else {
            return(max(l))
        }
    }))
    for(inf in sp$info) {
        max_t = max(max_t, inf$start_time + inf$duration)
    }
    return(max_t)
}
