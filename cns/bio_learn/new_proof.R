

tau = 10
thr = 0.5

neuron_solve = function(t_in, weights) {
    in_part = sum(exp(t_in/tau) * weights)
    
    tau * log( (tau * in_part)/(-thr + tau * in_part))
}


t_in = c(25, 25)
weights = c(1.0, 1.0)
w_seq = seq(0, 1, length.out=100)

tk = sapply(w_seq, function(w_s) neuron_solve(t_in, c(w_s, w_s)))

#t = seq(0, 100, length.out = 1000)
#plot(t, neuron_solve(t_in, weights), type="l")

plot(w_seq, tk, type="l")