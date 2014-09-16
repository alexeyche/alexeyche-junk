


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



gr_pl = function(m) {
  levelplot(m, col.regions=colorRampPalette(c("black", "white")))
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
