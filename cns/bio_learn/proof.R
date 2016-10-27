
Heavyside = Vectorize(function(x) {
    if (x > 0) {
        1.0
    } else {
        0.0
    }
})


require(LambertW)
Lamb = LambertW::W

tau_plus = 10.0
A_plus = 1.0
tau = 10.0
thr = 0.5


W = Vectorize(function(s) {
    Heavyside(s) * A_plus * exp(-s/tau_plus)
})

dw = function(t_in, t_o, t_d) {
    A_plus * exp((-t_d + t_in)/tau_plus) - A_plus * exp((-t_o + t_in)/tau_plus)
}

eta = Vectorize(function(s, w) {
    Heavyside(s) * w * (s/tau) * exp(1.0 - s/tau)
}, "s")



t_o = function(t_in, w) {
    -tau * Lamb(-thr * exp(-1) / w) + t_in
}

w_opt = function(s) {
    (thr * tau / s) * exp(s/tau - 1.0)
}

beta_w = function(w) {
    exp((tau/tau_plus) * Lamb(-thr * exp(-1.0)/w))
}

dw_f = function(s, w) {
    A_plus * beta_w(w_opt(s))  - A_plus * beta_w(w)
}

t = seq(0.0, 100, length.out=1000)

s = seq(1.0, 30.0, length.out=1000)

    

t_in = 10.0
t_d = 15
w_min = thr
w = w_min*5.0
lrate = 0.1


plot(s, eta(s, w), type="l")
plot(s, dw_f(s, w), type="l")



s = t_d - t_in
ww = c()
ee = c()
for (ep in 1:100) {
    w = w + lrate * dw_f(s, w)
    if (w < w_min) {
        w = w_min
    }
    
    e = abs(t_o(t_in, w) - t_d)^2
    
    ww = c(ww, w)
    ee = c(ee, e)
    cat("Error: ", e, "\n")
}


plot(ww, type="l")
par(new=TRUE)
plot(ee,type="l",col="red",xaxt="n",yaxt="n",xlab="",ylab="")
axis(4)
mtext("Error",side=4,line=3)



eta_vec = function(t, t_in, w) {
    sum(Heavyside(t-t_in) * w * ((t-t_in)/tau) * exp(- (t-t_in)/tau)/exp(-1.0))
}

te_x_exp = Vectorize(function(x, n=5) {
    s = 0
    for (ni in 1:n) {
        s = s + (x^ni)/factorial(ni-1)
    }
    # if ((x < 0) && (s > 0)) {
    #     return(0)
    # }
    return(s)
}, "x")


eta_vec_te = Vectorize(function(t, t_in, w) {
    sum(-Heavyside(t-t_in) * w * te_x_exp(-(t-t_in)/tau, 14)/exp(-1.0))
}, "t")


t = seq(0.0, 250.0, length.out=1000)
ee = eta_vec_te(t, c(15, 60), c(0.5, 0.5))

plot(t, ee, type="l")

# 
# x=-(t-10)/tau
# #plot(t, -Heavyside(-x)*x*exp(x), type="l")
# plot(t, -Heavyside(-x)*te_x_exp(x,n=50), col="red", type="l")
