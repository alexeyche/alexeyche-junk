tau_plus = 10.0
A_plus = 1.0
tau = 10.0
thr = 0.5

Heavyside = Vectorize(function(x) {
    if (x > 0) {
        1.0
    } else {
        0.0
    }
})

cut_exp = Vectorize(function(x) {
    if (x > 0.0) {
        return(0.0)
    }
    return(exp(x))
})

cut_log = Vectorize(function(x) {
    if (x > 1.0) {
        return(Inf)
    }
    return(log(x))
})


t_o = function(t_in, w) {
    tau * (cut_log(sum(w * cut_exp(t_in/tau)/thr)))
}

eta = Vectorize(function(t, t_in, w) {
    exp(-t/tau) * sum(Heavyside(t-t_in) * w * exp(t_in/tau))
}, "t")

t_in = c(10, 15, 25)
w = c(1.0, 1.0, 1.0)

t = seq(0, 100, length.out=1000)


# w * exp(-(t-t_in)/tau) = thr
# t = - log(thr/w)*tau + t_in

plot(t, eta(t, 25, 1.0), type="l")
