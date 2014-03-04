

grab_epsp <- Vectorize(function(t, sp) {
    sum(epsp(t-sp))
},"t")

  

p_stroke <- Vectorize(function(u) {
    beta/(1+ exp(alpha*(tr-u))) 
})


ratecalc = function(w) {
  w4 = w^6
  0.04* w4/(w4+ws^6) 
}

#ww = seq(0,20, by=0.1)
#plot(ww, ratecalc(ww), type="l")