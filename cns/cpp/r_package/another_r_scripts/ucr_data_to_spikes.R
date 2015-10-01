
require(Rdnn)
setwd(path.expand("~/dnn/r_scripts"))

source("ucr_ts.R")
source("env.R")


norm = function(x) {
    x/sqrt(sum(x^2))
}

ucr_data_to_spikes = function(
    N
  , sample_size
  , dt
  , gap_between_patterns = 0
  , data_name = synth
  , sel =c(1:300) #c(1:10, 51:60, 101:110, 151:160)
  , znorm = FALSE
  , vec_norm = FALSE
) {
    if(znorm && vec_norm) {
        stop("Need to choose one normalization method")
    }
    ts_dst_dir= TS_PLACE
    ts_dir = pj(DATASETS_PLACE, "ucr")
    
    dir.create(ts_dst_dir, FALSE, TRUE)        
    #dir.create(dst_dir, FALSE, TRUE)
    
    c(data_train, data_test) := read_ts_file(data_name, sample_size, ts_dir)
    min_train = min(sapply(data_train, function(x) min(x$data)))
    min_test = min(sapply(data_test, function(x) min(x$data)))
    min_val = min(min_train, min_test)
    
    max_train = max(sapply(data_train, function(x) max(x$data)))
    max_test = max(sapply(data_test, function(x) max(x$data)))
    max_val = max(max_train, max_test)
    intercept = seq(min_val, max_val, length.out=N)
    data_complect = list(train=data_train, test=data_test)
    spikes_complect = list()
    if(znorm) {
        data_name = sprintf("%s_znorm", data_name)
    } else
    if(vec_norm) {
        data_name = sprintf("%s_vec_norm", data_name)
    }
    for(data_part in names(data_complect)) {
        ts = data_complect[[data_part]]
        
        sp = list(ts_info=list(labels_timeline = NULL, unique_labels = NULL))
        
        sp$values = lapply(1:N, function(i) x <- vector(mode="numeric", length=0))
        
        labels = NULL        
        time = 0
        for(i in sel) {
            if(znorm) {
                ts[[i]]$data = (ts[[i]]$data - mean(ts[[i]]$data))/sd(ts[[i]]$data)
            } else
            if(vec_norm) {
                ts[[i]]$data = norm(ts[[i]]$data)
            }
            
            for(x in ts[[i]]$data) {
                d = abs(x - intercept)
                ni = which(d == min(d))
                sp$values[[ni]] = c(sp$values[[ni]], time)
                time = time + dt
            }
            time = time + gap_between_patterns
            sp$ts_info$labels_timeline = c(sp$ts_info$labels_timeline, time)
            labels = c(labels, as.character(ts[[i]]$label))
        }
        sp$ts_info$unique_labels = unique(labels)
        
        l_ids = list()
        idx = 0
        for(l in sp$ts_info$unique_labels) {
            l_ids[[l]] = idx
            idx = idx + 1
        }
        sp$ts_info$labels_ids = as.numeric(sapply(labels, function(l) l_ids[[l]]))
        spikes_complect[[data_part]] = sp

        fname = sprintf("%s/%s_%s_len_%s_classes_%s.pb", ts_dir, data_name, length(sel), length(sp$ts_info$unique_labels), data_part)
        RProto$new(fname)$write(sp, "SpikesList")
        fname = sprintf("%s/%s_%s_len_%s_classes_%s.pb", ts_dst_dir, data_name, length(sel), length(sp$ts_info$unique_labels), data_part)
        ts_data = c(sapply(ts[sel], function(x) x$data))
        #ts_data = norm(ts_data)
        RProto$new(fname)$write(
            list(
                values=ts_data
              , ts_info=sp$ts_info
            )
            , "TimeSeries"
        )
    }
    return(spikes_complect)
}
