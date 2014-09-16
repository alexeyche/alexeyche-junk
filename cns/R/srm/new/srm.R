tm = 15
dt = 1


rect = function(t) {
    if(abs(t)>0.5) return(0)
    if(abs(t)<0.5) return(1)
    return(0.5)
}

epsp <- Vectorize(function(s) {
    if((s<0)||(s == Inf)) { return(0) }
    exp(-s/tm)
})

u_abs <- -100 # mV
u_r <- -50#-50 # mV
trf <- 3.25 # ms
trs <- 3 # ms
dr <- 1 # ms

nu <- Vectorize(function(s) {
    if(s<0) { return(0)}
    if(s<dr) { return(u_abs) }
    u_abs*exp(-(s+dr)/trf)+u_r*exp(-s/trs)
})


lambda0 = 10.0
probf <- Vectorize(function(u, ut) {
    lambda0 * exp( (u-ut)/dt )
})

b1 = 12.5 # mV
t1 = 37.22 # ms
b2 = 1.98 # mV
t2 = 499.80
gamma_tr = Vectorize(function(t) {
    if(t<0) return(0)
    b1*exp(-t/t1) + b2*exp(-t/t2)
})

v0 = 1

v_tr = function(t, sp) {
    v0 + gamma_tr(t-sp) #+ nu(t-sp)
}


u_rest = -70
srm = function(t, sp, syn_spikes, w) {
    u = u_rest
    u = u + sum(w*sapply(syn_spikes, function(tf) sum(epsp(t-tf))))
    u = u + sum(nu(t-sp))
    return(u)
}

uacc = NULL

syn_spikes = list(c(10,20,30), c(12,22,33,44))
w = c(10, 10)

Tmax=1000 # ms
for(t in seq(0,Tmax, by=dt)) {
    u = srm(t, sp, syn_spikes, w)
    #probf(u)
    uacc = c(uacc, u)
}
plot(uacc, ylim=c(-75, 0), type="l")




