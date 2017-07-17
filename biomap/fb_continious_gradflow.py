

import numpy as np
from fb_common import Sigmoid, Linear, oja_rule, norm, Learning
from util import shl, shm, shs
import os
from os.path import join as pj
from matplotlib import pyplot as plt
from matplotlib.pyplot import cm

def smooth(signal, sigma=0.01, filter_size=50):
    lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
    return np.convolve(lf_filter, signal, mode="same")

np.random.seed(3)

act = Linear()

in_size = 3
hidden_size0 = 1
fb_size0 = 1
out_size = 1


T = 100

x_s = np.zeros((T, in_size))

x_s[25,0] = 1.0
x_s[50,1] = 1.0
x_s[75,2] = 1.0
x_s = np.asarray([smooth(x_s[:, ni]) for ni in xrange(in_size)]).T


y_t_s = np.zeros((T, out_size))
y_t_s[50,0] = 1.0
y_t_s = np.asarray([smooth(y_t_s[:, ni]) for ni in xrange(out_size)]).T



U0 = np.zeros((hidden_size0,))
U1 = np.zeros((out_size,))

F0 = np.zeros((fb_size0,))


W0 = -0.1 + 0.2*np.random.rand(in_size, hidden_size0)
W1 = -0.1 + 0.2*np.random.rand(hidden_size0, out_size)

W0_start, W1_start = W0.copy(), W1.copy()

B0 = -0.1 + 0.2*np.random.randn(out_size, fb_size0)
FB0 = -0.1 + 0.2*np.random.randn(fb_size0, hidden_size0)

stats_ep = []

lrate = 1e-01  # BP, FA
mean_lambda = 0.1
        
# lrate = 5e-02

lrule = Learning.HEBB
epochs = 1

U0m = np.zeros(U0.shape)

for lrule in (Learning.BP, Learning.FA, Learning.HEBB):
    slice_size = 50

    W0a = np.linspace(-2.0, 2.0, slice_size)
    W1a = np.linspace(-2.0, 2.0, slice_size)

    W0res = np.zeros((slice_size, slice_size))
    W1res = np.zeros((slice_size, slice_size))
    dW0res = np.zeros((slice_size, slice_size))
    dW1res = np.zeros((slice_size, slice_size))
    error_res = np.zeros((slice_size, slice_size))

    W0_idx = (1, 0)
    W1_idx = (0, 0)

    for ri, W0_v in enumerate(W0a):
        for ci, W1_v in enumerate(W1a):

            records = []
            stats = []
            
            fb0_s = np.zeros((T, fb_size0))
            W0[W0_idx] = W0_v        
            # W0[W1_idx] = W1_v
            W1[W1_idx] = W1_v

            for ti in xrange(T):
                x = x_s[ti, :]
                fb0 = fb0_s[ti, :]
                y_t = y_t_s[ti, :]

                U0_ff = np.dot(x, W0)
                U0_fb = np.dot(fb0, FB0)

                U0 = U0_ff + U0_fb 
                
                
                a0 = act(U0)
                
                U1 = np.dot(a0, W1)
                y = act(U1)

                e = y - y_t
                error = 0.5 * np.inner(e, e)

                F0 = np.dot(e, B0)
                
                next_idx = min(ti+1, T-1)
                fb0_s[next_idx, :] = F0.copy()

                if lrule == Learning.BP or lrule == Learning.FA:
                    if lrule == Learning.BP:
                        dh0 = np.dot(W1, e) * act.deriv(a0).T
                    else:
                        dh0 = F0 * act.deriv(a0)

                    dW0 = - np.outer(x, dh0)
                    dW1 = - np.outer(a0, e)
                    
                    # dW0 = norm(dW0)
                    # dW1 = norm(dW1)

                elif lrule == Learning.HEBB:
                    dW0 = oja_rule(x, U0_fb, W0, act.deriv(a0))
                    
                    dW1 = - np.outer(a0, e)

                    dW0 = np.sign(W0) * dW0
                    
                    # dW0 = norm(dW0)
                    # dW1 = norm(dW1)
                else:
                    raise NotImplementedError

                records.append(
                    tuple(
                        v.copy() for v in (U0, a0, U1, y, F0, dW0, dW1, U0_ff, U0_fb)
                    )
                )
                
                stats.append(
                    tuple(
                        v.copy() for v in (e, error)
                    )
                )

            es = np.asarray([r[0] for r in stats])
            error_s = np.asarray([r[1] for r in stats])


            U0s = np.asarray([r[0] for r in records])
            a0s = np.asarray([r[1] for r in records])
            U1s = np.asarray([r[2] for r in records])
            ys = np.asarray([r[3] for r in records])
            F0s = np.asarray([r[4] for r in records])
            dW0s = np.asarray([r[5] for r in records])
            dW1s = np.asarray([r[6] for r in records])
            U0_ffs = np.asarray([r[7] for r in records])
            U0_fbs = np.asarray([r[8] for r in records])
            
            W0res[ri, ci] = W0_v
            W1res[ri, ci] = W1_v
            dW0res[ri, ci] = np.sum(dW0s, 0)[W0_idx]
            dW1res[ri, ci] = np.sum(dW1s, 0)[W1_idx]
            # dW1res[ri, ci] = np.sum(dW0s, 0)[W1_idx]
            error_res[ri, ci] = np.mean(error_s)


    plot = plt.figure(figsize=(20,10))
    plt.quiver(
        W0res, W1res, dW0res, dW1res,
        error_res,
        cmap=cm.seismic,     # colour map
        headlength=7, headwidth=5.0)        # length of the arrows

    plt.colorbar()                      # add colour bar on the right
    # plt.show(plot)        
    plot.savefig(pj(os.environ["HOME"], "tmp", "{}.png".format(lrule)))
    plot.clf()

    print lrule