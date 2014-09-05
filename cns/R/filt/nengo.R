dt = 1

t_rc = 0.02 * 1000
t_ref = 0.002 * 1000
M = 100
rate_low = 10
rate_hight = 100
v_tresh = 1

generate_gain_and_bias = function(count, intercept_low, intercept_high, rate_low, rate_high) {
    gain = bias = NULL
    intercept = intercept_low+(intercept_high-intercept_low)*runif(count)
    rate = rate_low +(rate_high-rate_low)*runif(count)
    z = 1.0 / (1-exp( (t_ref - (1/rate))/t_rc ) )
    g = (1-z)/(intercept - 1.0)
    b = 1 - g*intercept
    return(list(g, b))
}

c(gain, bias) := generate_gain_and_bias(M, -1, 1, rate_low, rate_high)

n = list(v=rep(0, M), ref=rep(0,M))

run_neurons = function(input) {
    v = ref = rep(0, M)
    dV = dt * (input - v)/t_rc
    n$v <<- n$v + dV
    n$v[ n$v<0 ] <<- 0
    
    in_refr = n$ref>0
    n$v[in_refr] <<- 0
    n$ref[in_refr] <<- n$ref[in_refr] - dt
    
    fired = n$v>v_tresh
    n$v[fired] <<- 0
    n$ref[fired] <<- t_ref
    return(fired)    
}

encoder = sample(c(1,-1),M, replace=TRUE)

x_ts = loadMatrix("~/prog/sim/ts/synthetic_control/synthetic_control_TRAIN_120",1)[1,]

spikes = NULL
for(i in 1:length(x_ts)) {
    x = x_ts[1]
    input = x * encoder * gain + bias
    
    current_spikes = run_neurons(input)
    spikes = cbind(spikes, as.integer(current_spikes))
    
}    