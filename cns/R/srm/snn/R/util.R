

get_unique_ids <- function(n) {
  if(!exists('ID_MAX')) { assign('ID_MAX', 0, envir=.GlobalEnv) }
  id_max = get('ID_MAX',.GlobalEnv)
  assign('ID_MAX', id_max+n, envir=.GlobalEnv)
  return((id_max+1):(id_max+n))
}


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

list_to_matrix = function(l) {
  maxw_len = 0
  invisible(sapply(l, function(n) maxw_len<<-max(maxw_len, length(n))))
  m = matrix(0, nrow=length(l), ncol=maxw_len)
  for(sp_i in 1:length(l)) {
    m[sp_i,] = c(l[[sp_i]], rep(0, maxw_len - length(l[[sp_i]]) ) )
  }
  return(m)
}

blank_net = function(N) {
    if(N<=0) return(list())
    net = list()
    for(i in 1:N) {
        net[[i]] = numeric(0)
    }
    return(net)
}

make_conn_matrix = function(layers) {
    max_conn_id = -1
    min_conn_id = Inf
    for(neurons in layers) {
        invisible(sapply(neurons$id_conns(), function(cid) { 
            if(length(cid) !=0) {
                max_conn_id <<-max(max_conn_id, max(cid))
                min_conn_id <<-min(min_conn_id, min(cid))
            }
        }))
    }
    conn_m = matrix(0, nrow=max_conn_id, ncol=sum(sapply(layers, function(n) n$len()))) 
    return(conn_m)
}


get_stat_matrix <- function(neurons, stat_list) {
    stat_m = make_conn_matrix(list(neurons))      
    for(ni in 1:neurons$len()) {
        for(syn_num in 1:length(neurons$id_conns()[[ni]])) {
            stat_m[ neurons$id_conns()[[ni]][syn_num] , ni] = stat_list[[ni]][syn_num]
        }
    }
    return(stat_m)
}

gr_pl = function(m) {
  levelplot(m, col.regions=colorRampPalette(c("black", "white")))
}

saveModelToFile = function(n, model_file) {
    l = list()
    l[[1]] = get_stat_matrix(n, n$obj$W)
    l[[2]] = get_stat_matrix(n, n$obj$syn_spec)
    l[[3]] = get_stat_matrix(n, n$obj$syn_del)
    l[[4]] = n$obj$axon_del
    saveMatrixList(model_file, l)
}

loadModelFromFile = function(n, model_file) {
  if(file.exists(paste(model_file, ".idx", sep=""))) {  
    W = loadMatrix(model_file, 1)
    syn_spec = loadMatrix(model_file, 2)
    syn_del = loadMatrix(model_file, 3)
    axon_del = loadMatrix(model_file, 4)
    
    n$obj$axon_del = axon_del
    invisible(sapply(1:N, function(id) { 
      id_to_conn = which(W[,id] != 0)
      n$obj$W[[id]] <<- W[id_to_conn, id] 
      n$obj$id_conns[[id]] <<- id_to_conn
      n$obj$syn_spec[[id]] <<- syn_spec[id_to_conn, id]
      n$obj$syn_del[[id]] <<- syn_del[id_to_conn, id]
    }))  
    cat("Load - Ok\n")
  } else {
    cat("Can't find file for model ", model_file, "\n")
  }
  
}

getSpikesFromMatrix = function(sp) {
    net = blank_net(nrow(sp))
    for(i in 1:length(net)) {
        spike_elems = which(sp[i,]>0)
        if(length(spike_elems)>0) {
            if(sp[i,1] == 0) {
                spikes_elems = c(1, spike_elems)
            }
        }
        net[[i]] = sp[i, spike_elems]
    }
    return(net)
}

get_const = function(const_name) {
    s = system(sprintf("egrep -o '%s[ ]*=[ ]*[ \\/_.a-zA-Z0-9]+' %s | cut -d '=' -f 2 ", const_name, const_ini), intern=TRUE)
    s_arr = strsplit(s, " ")[[1]]
    s_out = NULL
    for(s_el in s_arr) {
        if(s_el != "") {
            s_out =c(s_out, s_el)
        }
    }
    return(s_out)
}
patch_const = function(const_ini, param, new_val) {
    system( sprintf("sed -i -e 's:%s[ ]*=.*:%s = %s:g' %s", param, param, new_val, const_ini))
}
