

plot_act <- function(x, treshold=30) {
  V_gr <- NULL
  for(i in 1:nrow(x)) {  
    spike_occ <- which(x[i,] >= treshold)
    V_gr <- rbind(V_gr, cbind(spike_occ, rep(i, length(spike_occ))))
  }
  plot(V_gr, pch=5, cex=0.5 )
}