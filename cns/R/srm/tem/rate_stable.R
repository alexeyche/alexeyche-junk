
source('encode.R')

dt = 0.1
t_rc = 2
t_ref = 0.2

u = rep(0.1, 1000)

samp_ids = 1:600 #sample(300, 300)

levels_char = list()

g_lims = list(c(10, 1500), c(5,1000), c(1,300), 
              c(0.5, 100), c(0.5, 100), c(0.5, 100),
              c(0.5, 100), c(0.5, 100), c(0.5, 100))

for(nlev in 1:9) {
  u = sapply(wave_all[samp_ids], function(x) x@W[[nlev]])
  Tmax = length(u)*dt
  g0 = g_lims[[nlev]][1]
  gmax = g_lims[[nlev]][2]
  gs = seq(g0, gmax, length.out=100)
  
  rates = matrix(0, length(gs), ncol(u))
  for(ui in 1:ncol(u)) {
    for(gi in 1:length(gs)) {
      sp = lif_encode(u[,ui]*gs[gi], dt, t_rc, t_ref)
      if(length(sp)>128) {
        rates[gi, ui] = Inf
      } else {
        rates[gi,ui] = length(sp)
      }
    }
  }
  levels_char[[nlev]] = list(r=rowMeans(rates), gs=gs)
  cat("nlev: ", nlev, " done \n")
}
nlev=1
plot(levels_char[[nlev]]$gs, levels_char[[nlev]]$r, type="l")

all_g = NULL
targ_rate = 20
for(nlev in 1:9) {
  gs = levels_char[[nlev]]$gs
  r = levels_char[[nlev]]$r
  gs_f = gs[which(r<=targ_rate)]
  g = (gs_f[length(gs_f)]+ gs_f[length(gs_f)-1])/2
  all_g = c(all_g, g)
}
cat(all_g,sep=",")

#sp = lif_encode(u[,ui]*g, dt, t_rc, t_ref)
#Tmax = length(u[,ui])*dt
#length(sp)/Tmax

#par(mfrow=c(2,1))
#plot(seq(0,Tmax,length.out =length(u[,ui])), u[,ui], xlim=c(0,Tmax), type="l")
#plot(sp, rep(1,length(sp)), xlim=c(0,Tmax))
