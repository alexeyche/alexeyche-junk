


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
    xyplot(y ~ x, list(x = x, y = y), xlim=c(T0, max(x)), cex=cex*10,  col = "black")
}

plot_rastl <- function(raster, lab="",T0=0, Tmax=Inf, i=-1, plen=-1) {
    if( "t" %in% names(raster)) {
        return(prast_mpl(raster, T0, Tmax))
    }
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

gr_pl = function(m) {
  levelplot(m, col.regions=colorRampPalette(c("black", "white")))
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
