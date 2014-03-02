
iaf_encode = function(u, dt, b, d, sd_tr, R, C) {
  y = 0
  interval = 0
  sp = NULL
  
  int_meth = 'trapz'
  last = length(u)
  if(is.infinite(R)) {
      if(int_meth == 'quad') {
        comp_y = function(y, i) y + dt*(b+u[i])/C
      } else
        if(int_meth == 'trapz') {
          comp_y = function(y, i) y + dt*(b+(u[i]+u[i+1])/2)/C
          last = last-1
        }
  } else {
     RC = R*C;
     comp_y = function(y,i) y*exp(-dt/RC)+R*(1-exp(-dt/RC))*(b+u[i])
  }

  dnew = d+rnorm(1, 0, sd_tr)
  for(i in 1:last) {
    y = comp_y(y, i)
    interval = interval + dt
    if(y>=dnew) {
      sp = c(sp, interval)
      y = y - dnew
      dnew = d+abs(rnorm(1, 0, sd_tr))
    }
  }
  return(sp)
}

lif_encode = function(u, dt, t_rc, t_ref) {
  v = 0
  ref = 0
  sp = NULL
  t = 0
  for(i in 1:length(u)) {
    t = t + dt
    v = v + dt*(u[i]-v)/t_rc
    if(v<0) v = 0
    if(ref>0) {
      v = 0
      ref = ref - dt
    }
    if(v>1) {
      v = 0
      ref = t_ref
      sp = c(sp, t)
    }
  }
  return(sp)
}
