#!/usr/bin/RScript

require(Matrix)
source('plots.R')

dyn.load("/home/alexeyche/my/git/alexeyche-junk/cns/socket_test/sim_socket_r.so")
#dyn.load("/home/alexeyche/prog/alexeyche-junk/cns/socket_test/sim_socket_r.so")

run_server <- function(port = 7778) {
    .Call("r_run_server", as.integer(port))    
}

get_message <- function(mc_p) {
    x <- .Call("r_get_message",mc_p)
    if(! is.null(x)) {
        i_index <- rep(seq(1:x$nrow), x$ncol)
        j_index <- c(sapply(seq(1:x$ncol),function(j) { rep(j, x$nrow) }))
        assign(x$name, sparseMatrix(i = i_index,j = j_index,x = x$x), envir = .GlobalEnv)
    } else {
        return(NULL)
    }
}

mc_p <- run_server()

load <- function() {
    x <- get_message(mc_p)
    while(! is.null(x)) {
        x <- get_message(mc_p)
    }
}
