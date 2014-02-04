

grab_epsp <- Vectorize(function(t, sp) {
    sum(epsp(t-sp))
},"t")

  

p_stroke <- Vectorize(function(u) {
    beta/(1+ exp(alpha*(tr-u))) 
})
