N = 10

inp_spikes = matrix(0, sample_duration, N)
sample_duration = 10

for(i in 1:sample_duration) { # gen pattern
    ni =  i %% N
    inp_spikes[i, ni+1] = 1.0
#    inp_spikes[[i]] = rpois(sample_duration, rates[i]/1000)
}


alpha = 0.1
dt = 0.05

w = 10.0*rnorm(N)
ss = NULL
s = 0.0
for (istep in 1:sample_duration) {
    delta = (1.0 - cos(s) + (1.0 + cos(s)) * inp_spikes[istep,] %*% w)     
    
    #new_s = (1.0 - alpha) * s + alpha * delta
    new_s = s + dt * delta     
    
    ss = c(ss, new_s)
    s = new_s
}

plot(ss,type="l")