
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
alpha <- 0.75
beta <- 0.75
tr <- -50 # mV

# srm
u_rest <- -70 # mV


# input spike train
duration = 300

# net cfg

N = 10
M = 50
edge_prob = 0.5

# start weights
start_w.M = 10 #matrix(rnorm( M*N, mean=2, sd=0.5), ncol=N, nrow=M)
start_w.N = 5 #matrix(rnorm( (N-1)*N, mean=2, sd=0.5), ncol=N, nrow=(N-1))

# learning
llh_depr = 0.04
lr = 0.025

refr_mode = 'middle'
