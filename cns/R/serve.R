#!/usr/bin/RScript

dyn.load("/home/alexeyche/prog/alexeyche-junk/cns/cpp/socket/sim_socket_r.so")

create_server <- function(port = 7778) {
    .Call("r_run_server", as.integer(port))
}

read_message <- function(mc_p) {
  x <- .Call("r_get_message", mc_p)  
  if(! is.null(x)) {
    assign(x$name, x$x, envir = .GlobalEnv)
    return(TRUE)
  } else {
    return(NULL)
  }  
}
mc_p <- create_server()

load <- function() {
    x <- read_message(mc_p)
    while(! is.null(x) ) {
        x <- read_message(mc_p)    
    }
}

