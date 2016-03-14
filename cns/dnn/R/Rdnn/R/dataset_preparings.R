

# sample_size = 60
#  data_name = UCR.SYNTH
#  sel = NULL
#  gap_between_patterns = 0
#  dt = 1.0
#  save_on_disk=TRUE
#  use_cache=TRUE

add.to.list = function(l, v) {
    l[[length(l)+1]] = v
    return(l)
}

prepare.ucr.data = function(
    sample_size = 60
  , data_name = UCR.SYNTH
  , sel = NULL
  , gap_between_patterns = 0
  , dt = 1.0
  , save_on_disk=TRUE
  , use_cache=TRUE
) {

    c(data_train, data_test) := read_ucr_file(data_name, sample_size)
    data_complect = list(train=data_train, test=data_test)
    
    data_out = list()
    for(data_part in names(data_complect)) {
        ts = data_complect[[data_part]]
        sel.w = sel
        if(is.null(sel.w)) {
            sel.w = 1:length(ts)    
        }
        
        fname = sprintf(
            "%s/%s_%snum_%slen_%sclasses_%sgap_%s.pb"
          , ts.path()
          , data_name
          , length(sel.w)
          , sample_size
          , length(unique(sapply(ts, function(x) x$label)))
          , gap_between_patterns
          , data_part
        )
        if(use_cache && file.exists(fname)) {
            data_out[[data_part]] = proto.read(fname)
            cats("Using cached time series from %s\n", fname)
        } else {
            ts_info = list()
            labels = c()
            
            time = 0
            ts_data = c()
            for(i in sample(sel.w)) {
                ts_info = add.to.list(
                    ts_info, 
                    list(start_time = time, label = as.character(ts[[i]]$label), duration = length(ts[[i]]$values))
                )                
                for(x in ts[[i]]$values) {
                    time = time + dt    
                }
                time = time + gap_between_patterns
                
                ts_data =c(ts_data, ts[[i]]$values, rep(0, gap_between_patterns))
            }
            data_out[[data_part]] = time.series(values=matrix(ts_data, nrow=1, ncol=length(ts_data)), info=ts_info)
            if(save_on_disk) {
                cats("Saving %s in %s\n", data_part, fname)
                proto.write(data_out[[data_part]], fname)
            }
        }
    }
    return(data_out)
}


read_ucr_file <- function(ts_name, sample_size, ucr_dir=ds.path("ucr")) {
    process_datamatrix <- function(m) {
        l = ncol(m)
        out = list()
        for(ri in 1:nrow(m)) {
            out[[ri]] = list(values = m[ri,2:l], label = m[ri,1])    
        }  
        return(out)
    }
    if(!is.na(sample_size)) {
        ts_file_train = sprintf("%s/%s/%s_TRAIN_%s", ucr_dir, ts_name, ts_name, sample_size)
        ts_file_test = sprintf("%s/%s/%s_TEST_%s", ucr_dir, ts_name, ts_name, sample_size)
        if(!file.exists(ts_file_train)) {
            c(train_dataset, test_dataset) := read_ucr_file(ts_name, NA, ucr_dir)
            train_dataset_inter = matrix(0, length(train_dataset), sample_size+1)
            test_dataset_inter = matrix(0, length(test_dataset), sample_size+1)
            for(i in 1:length(train_dataset)) {
                inter_ts = interpolate_ts(train_dataset[[i]]$values, sample_size)
                train_dataset_inter[i, ] = c(train_dataset[[i]]$label,inter_ts)
            }
            for(i in 1:length(test_dataset)) {
                inter_ts = interpolate_ts(test_dataset[[i]]$values, sample_size)
                test_dataset_inter[i, ] = c(test_dataset[[i]]$label,inter_ts)
            }
            write.table(train_dataset_inter,file=ts_file_train,sep=" ", col.names = F, row.names = F, append=F)
            write.table(test_dataset_inter,file=ts_file_test,sep=" ", col.names = F, row.names = F)
        }
    } else {
        ts_file_train = sprintf("%s/%s/%s_TRAIN", ucr_dir, ts_name, ts_name)
        ts_file_test = sprintf("%s/%s/%s_TEST", ucr_dir, ts_name, ts_name)
    }
    nlines_train = as.numeric(system(sprintf("grep -c ^ %s", ts_file_train), intern=TRUE))
    nlines_test = as.numeric(system(sprintf("grep -c ^ %s", ts_file_test), intern=TRUE))
    ts_train = scan(ts_file_train)
    ts_test = scan(ts_file_test)
    
    ts_train = matrix(ts_train, nrow=nlines_train, byrow=TRUE)
    ts_test = matrix(ts_test, nrow=nlines_test, byrow=TRUE)
    return( list(process_datamatrix(ts_train), process_datamatrix(ts_test)) )
}

UCR.SYNTH = "synthetic_control"
UCR.ECG = "ECG200"
UCR.FACE = "FaceAll"
UCR.STARLIGH = "StarLightCurves"

empty.ts = function() {
    time.series(values=c(), info=list())
}

empty.spikes = function(N=NULL) {
    l = list()
    if(!is.null(N)) {
        l = lapply(1:N, function(i) numeric(0))
    }
    spikes.list(values=l, info=list())
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


intercept.data.to.spikes = function(ts, N, dim_idx, dt=1, gap_between_patterns=0) {
    data = ts$values[dim_idx, ]
    
    min_val = min(data)
    max_val = max(data)
    
    intercept = seq(min_val, max_val, length.out=N)
    
    sp = empty.spikes(N)
    
    src_lab_times = sapply(ts$info, function(i) i$start_time + i$duration)
    
    
    time = 0
    i = 0
    
    for(x in data) {
        if((i == head(src_lab_times, n=1))&&(length(src_lab_times)>1)) {
            time = time + gap_between_patterns            
            src_lab_times = src_lab_times[-1]
            ts_info = head(ts$info, n=1)
#            sp$info = add.to.list(sp$info, list(start_time=time))
        }   
        d = abs(x - intercept)
        ni = which(d == min(d))
        sp$values[[ni]] = c(sp$values[[ni]], time)
        
        time = time + dt        
        i = i + 1
    }
    
    lab_times = ts$ts_info$labels_timeline
    lab_times = lab_times * dt
    lab_times = lab_times + gap_between_patterns * 1:length(lab_times)    
    sp$ts_info = ts$ts_info
    sp$ts_info$labels_timeline = lab_times
    return(sp)
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
