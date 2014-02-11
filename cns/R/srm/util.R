

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

resample <- function(x, ...) x[sample.int(length(x), ...)] 

get_weights_matrix_old <- function(neurons) {
  W = lapply(neurons, function(n) n$w)
  maxw_len = 0
  invisible(sapply(neurons, function(n) maxw_len<<-max(maxw_len, length(n$w))))
  W = sapply(W, function(row) { c(row, rep(0, maxw_len-length(row)))} )
  return(W)
}

get_weights_matrix <- function(layers) {
  max_conn_id = -1
  min_conn_id = Inf
  for(neurons in layers) {
    invisible(sapply(neurons$id_conns, function(cid) { 
      if(length(cid) !=0) {
        max_conn_id <<-max(max_conn_id, max(cid))
        min_conn_id <<-min(min_conn_id, min(cid))
      }
    }))
  }
  W = matrix(0, nrow=max_conn_id, ncol=sum(sapply(layers, function(n) n$len)))
  
  n = layers[[1]]
  for(ni in 1:n$len) {
    for(syn_num in 1:length(n$id_conns[[ni]])) {
      W[ n$id_conns[[ni]][syn_num] , ni] = n$weights[[ni]][syn_num]
    }
  } 
  return(W)
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

get_unique_ids <- function(n) {
  if(!exists('ID_MAX')) { assign('ID_MAX', 0, envir=.GlobalEnv) }
  id_max = get('ID_MAX',.GlobalEnv)
  assign('ID_MAX', id_max+n, envir=.GlobalEnv)
  return((id_max+1):(id_max+n))
}

sp_in_interval = function(net, T0, Tmax) {
  nspikes = lapply(net, function(sp) { 
    left = findInterval(T0, sp)+1
    right = findInterval(Tmax+0.01, sp, rightmost.closed=TRUE)
    if(left<=right) sp[left:right]
  })  
  return(nspikes)
}
