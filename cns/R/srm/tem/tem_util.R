

require(zoo)

approx_ts = function(x, nmax)  {
    approx_data = rep(NA, nmax)                                      
    dt_d =nmax/length(x)
    curt=0
    for(ri in 1:length(x)) {
      curt = curt + dt_d
      ct = ceiling(signif(curt, digits=5))                                        
      approx_data[ct] = x[ri]
    }
    return(na.approx(approx_data))
}


