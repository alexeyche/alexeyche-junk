dyn.load('sim_socket_r.so')
server <- .Call("r_run_server", as.integer(7778))
