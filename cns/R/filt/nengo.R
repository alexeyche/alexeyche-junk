


generate_gain_and_bias = function(count, rate_low, rate_high) {
    intercept_low = -1
    intercept_high = 1
    gain = bias = NULL
    intercept = intercept_low+(intercept_high-intercept_low)*runif(count)
    rate = rate_low +(rate_high-rate_low)*runif(count)
    z = 1.0 / (1-exp( (t_ref/1000 - (1/rate))/ (t_rc/1000) ) )
    g = (1-z)/(intercept - 1.0)
    b = 1 - g*intercept
    return(list(g, b))
}



run_neurons = function(input, n) {
    dV = dt * (input - n$v)/t_rc
    n$v = n$v + dV
    n$v[ n$v<0 ] <- 0
    
    in_refr = n$ref>0
    n$v[in_refr] <- 0
    n$ref[in_refr] = n$ref[in_refr] - dt
    
    fired = n$v>v_tresh
    n$v[fired] <- 0
    n$ref[fired] <- t_ref
        
    return(list(n, fired))
}

compute_response = function(x, encoder, gain, bias, time_limit=0.5) {
    n = list(v=rep(0, M), ref=rep(0,M))
    n$v = runif(M) 
    count = rep(0, M)
    
    input = x * encoder * gain + bias
    for(i in seq(0, time_limit*1000, by=dt)) {
        c(n, spikes) := run_neurons(input, n)
        count[spikes] = count[spikes] + 1
    }
    return(count/time_limit)
}

plot_tuning_curves = function(encoder, gain, bias) {
    x = seq(-1, 1, length.out=100)
    resp = NULL
    for(xv in x) {
        r = compute_response(xv, encoder, gain, bias, 0.5)
        resp = cbind(resp, r)
    }
    
    cols = rainbow(nrow(resp))
    for(ni in 1:nrow(resp)) {
        if(ni==1) {
            plot(x, resp[ni,], type="l", col=cols[ni], ylim=c(min(resp), max(resp)))
        } else {
            lines(x, resp[ni,], col=cols[ni])
        }
    }
}