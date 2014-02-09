

g2 = function(u) {
  pr + (u -u_rest)*gain_factor
}

neuron = setRefClass("neuron", fields = list(w = "vector", id_conn = "vector", id = "vector"),
                     methods = list( 
                       u = function(t, net) {
                         USRM2(t, constants, id, id_conn, w, net)
                         #e_syn= sapply(net[id_conn], function(sp) sum(epsp(t-sp)))
                         #u_rest + sum(w*e_syn) + sum(nu(t-net[[id]]))
                       },
                       p = function(t, net) {
                         g(u(t,net))
                       },
                       get_id = function() {
                         id
                       }))

net <- vector("list", 101)
mp = NULL
dt=1
inp_rates = seq(0, 50, by=1)
for(inp_rate in inp_rates) {

  n1 = neuron(w = rep(0.4, 100), id_conn = 1:100, id = 101)
  net[1:100] = -Inf
  net[101] = -Inf
  uu = pp = c()
  for(t in seq(0, 10000, by=dt)) {
    fired = ((inp_rate/1000)*dt)>runif(100)  
    for(fi in which(fired == TRUE)) {
      net[[fi]] = c(net[[fi]], t)
    }
    curu = n1$u(t, net)
    uu <- c(uu, curu)
    curp = g2(curu)
    pp <- c(pp, curp)
    if( ( (curp/1000)*dt)> runif(1)) {
      net[[101]] = c(net[[101]], t)
    }
  }
  mp = c(mp, mean(pp))
  
  plot(density(diff(net[[101]][net[[101]]>0])))
  cat("inp_rate: ", inp_rate, " mean_rate: ", mean(pp) ,"\n")
}