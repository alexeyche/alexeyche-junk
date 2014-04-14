
#source('constants.R')
#source('serialize_to_bin.R')

library(snn)

T0 = 0
Tmax = 30000
dt=1
M=100
sim_dim=1000
T = seq(T0, Tmax, by=dt) 

Twhole = Tmax*100 # 50 min


dir2save = "~/prog/sim/rfd_files"
N=0
net = list()
for(i in 1:(M+N)) net[[i]] = numeric(0)
ID_MAX=0
gr1 = TSNeurons(M = M)

vmax=30
v0=1
gauss_rates = Vectorize(function(k,j) {
  ((vmax-v0)*exp(-0.01*( min( abs(j-k), 100-abs(j-k) ) )^2) + v0 )/sim_dim
},"j")

for(ep in 1:100) {
  for(i in 1:(M+N)) net[[i]] = numeric(0)
  for(curt in T) {
    if(curt %% 200 == 0) {
      mu = sample( seq(1, 100, by=1), 1)
    }
    syn_fired = (gauss_rates(mu, 1:M)*dt)>runif(M)
    for(fi in which(syn_fired==TRUE)) {
      net[[ gr1$ids()[fi] ]] = c(net[[ gr1$ids()[fi] ]], curt)
    }
  }
  spike_file = sprintf("%s/ep_%d_%4.1fsec", dir2save, ep, Tmax/sim_dim)
  saveMatrixList(spike_file, list(list_to_matrix(net), 
                                  matrix(Tmax),
                                  matrix(0))
  )
  
  
}
