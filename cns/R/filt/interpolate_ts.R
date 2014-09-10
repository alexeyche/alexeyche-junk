
require(zoo)

interpolate_ts = function(ts, interpolate_size) {
    out_approx = NA
    
    while(length(out_approx) != interpolate_size) {
        out = rep(NA, interpolate_size)
        iter <- 0
        for(i in 1:length(ts)) {
            iter = iter+length(out)/length(ts)
            ct = floor(signif(iter, digits=5))                                        
            out[ct] = ts[i]
        }
        out_approx = na.approx(out)
        ts = out_approx
    }
    
    return(out_approx)
}

