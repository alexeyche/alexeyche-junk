#!/usr/bin/env sage
v0 = 0
N = 100
M1 = N/2
M2 = N/2
nu = 10^-5
w_in = nu
w_out = -1.0475*nu
mu_in = 10

t0 = 10

var('t', 's')

# epsp ==================================
epsp_full(t) = (t/(t0^2))*exp(-t/t0)
func_null(t) = 0
epsp = Piecewise([ [(-infinity, 0), func_null], [(0, infinity), epsp_full] ])
# W =====================================
t_syn = 5 # ms
t_pos = 1 # ms
t_neg = 10 # ms
Apos = 1
Aneg = -1
t_pos_ = t_syn*t_pos/(t_syn+t_pos)
t_neg_ = t_syn*t_neg/(t_syn+t_neg)

W_ltp = exp(t/t_syn)*(Apos*(1-t/t_pos_) + Aneg*(1-t/t_neg_))
W_ltd = Apos*exp(-t/t_pos) + Aneg*exp(-t/t_neg)
W = Piecewise([ [(-1000,0), W_ltp ],
                [(0, 1000), W_ltd ],
              ])

plot(W, t, 1,10)
# lambda ===============================================
lambda_in(t) = mu_in*cos(t/1000*2*40*2*pi) + mu_in
#lambda_in(t) = 10

epsp_l = epsp.laplace(t, s)
lambda_in_l = lambda_in.laplace(t, s)

lambda_big = inverse_laplace( epsp_l*lambda_in_l, s, t)


#print integrate(lambda_big, -infinity, infinity)
