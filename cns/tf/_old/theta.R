N = 10

sample_duration = 100
inp_spikes = matrix(0, sample_duration, N)

for(i in 1:sample_duration) { # gen pattern
    ni =  i %% N
    inp_spikes[i, ni+1] = 1.0
#    inp_spikes[[i]] = rpois(sample_duration, rates[i]/1000)
}

cos.m = function(x) {
    1*cos(x)    
}

alpha = 0.1
dt = 0.25

w = rnorm(N)
ss = NULL
s = 0.0
for (istep in 1:sample_duration) {
    delta = (1.0 - cos.m(s) + (1.0 + cos.m(s)) * inp_spikes[istep,] %*% w)     
    
    #new_s = (1.0 - alpha) * s + alpha * delta
    new_s = s + dt * delta     
    
    ss = c(ss, new_s)
    s = new_s
}

plot(-cos(ss)/2.0 + 0.5,type="l")