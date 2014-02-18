

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