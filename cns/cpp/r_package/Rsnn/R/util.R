


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




require(lattice)

plot_rastl <- function(raster, lab="",T0=0, Tmax=Inf, i=-1, plen=-1) {
    x <- c()
    y <- c()
    if((i>0)&&(plen>0)) {
        T0=plen*(i-1)
        Tmax=plen*i
    }
    for(ni in 1:length(raster)) {
        rast = raster[[ni]]
        rast = rast[rast >= T0]
        rast = rast[rast < Tmax]
        x <- c(x, rast)
        y <- c(y, rep(ni, length(rast)))
    }
    return(xyplot(y~x,list(x=x, y=y), main=lab, xlim=c(T0, max(x)), col="black"))
}

prast = plot_rastl
