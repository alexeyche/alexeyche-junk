#!/usr/bin/RScript


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

# matlab way to sum
sum.row <- function(m) {
    apply(m,2,sum)
}
sum.col <- function(m) {
    matrix(apply(m,1,sum), ncol=1)
}

# matlab stuff
rep.row <- function(row,n) {
    matrix(rep(row,n),nrow = n, byrow=TRUE)
}

integer.base.b <- function(x, b=2){
    xi <- as.integer(x)
    if(any(is.na(xi) | ((x-xi)!=0)))
        print(list(ERROR="x not integer", x=x))
    N <- length(x)
    xMax <- max(x)	
    ndigits <- (floor(logb(xMax, base=2))+1)
    Base.b <- array(NA, dim=c(N, ndigits))
    for(i in 1:ndigits){#i <- 1
        Base.b[, ndigits-i+1] <- (x %% b)
        x <- (x %/% b)
    }
    if(N ==1) Base.b[1, ] else Base.b
}

scale.vector <- function(vector,up=1,down=0) {
    f.max <- max(vector)
    f.min <- min(vector)
    down+(up-down)*(vector-f.min)/(f.max-f.min)
}

scale <- function(data, up=1, down=0) {
    out <- NULL
    if(is.matrix(data)) {
        for(j in 1:ncol(data)) {            
            out <- cbind( out, scale.vector(data[,j],up,down) ) 
        }
        return(out)
    }
    if(is.vector(data)) {
        return(scale.vector(data,up,down))
    }
}
