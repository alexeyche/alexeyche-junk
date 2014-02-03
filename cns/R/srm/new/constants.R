
# epsp
e0 <- 1.3 # mV
ts <- 1 # ms
tm <- 10 # ms


# refractoriness
u_abs <- -150 # mV
u_r <- -50#-50 # mV
trf <- 3.25 # ms
trs <- 3 # ms
dr <- 1 # ms


# stochastic treshold
alpha <- 0.71
beta <- 0.72
tr <- -50 # mV

# srm
u_rest <- -70 # mV


# input spike train
duration = 300
dt = 0.5
# net cfg

N = 10
M = 50
edge_prob = 0.1

# start weights
start_w.M = 10 #matrix(rnorm( M*N, mean=2, sd=0.5), ncol=N, nrow=M)
start_w.N = 5 #matrix(rnorm( (N-1)*N, mean=2, sd=0.5), ncol=N, nrow=(N-1))

# learning
llh_depr = 0.2
lr = 1

refr_mode = 'high'

stdp_learning=TRUE
Aplus = 1.5
Aminus = 1.5
tplus = 10
tminus = 10
constants = list(dt=dt, e0=e0, ts=ts, tm=tm, u_abs=u_abs, u_r=u_r, trf=trf, trs=trs, 
                 dr=dr, alpha=alpha, beta=beta, tr=tr, u_rest=u_rest,
                 stdp_learning=stdp_learning, Aplus=Aplus, Aminus=Aminus, tplus=tplus, tminus=tminus)



