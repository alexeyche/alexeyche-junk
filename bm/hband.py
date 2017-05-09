
from numpy import log, argsort, ceil
import numpy as np

import sys
import os

from util import shl, shm, shs
import numpy as np
from datasets import get_toy_data
from os.path import join as pj

from models import Hopfield, ExpDecayHopfield
from activation import *
from cost import MseCost
from models import initialize_layer
from opt import *

def get_batch(d, idx, batch_size):
    return d[idx*batch_size:(idx+1)*batch_size]

def one_hot(y, y_size):
    y_oh = np.zeros((y.shape[0], y_size))
    y_oh[np.arange(y.shape[0]), y] = 1.0
    return y_oh



def get_random_hyperparameter_configuration():
    real_params = {
        "epsilon": (0.1, 2.0),
        "hidden_size": (10, 200),
        "lrate0": (1e-05, 0.1),
        "lrate1": (1e-05, 0.1),
        "n_neg": (1, 20),
        "n_pos": (1, 20),
        "beta": (0.01, 2.0)
    }
    coin = np.random.random(len(real_params))

    ans = dict([ 
        (k, min_v + coin[p_id] * (max_v - min_v)) 
        for p_id, (k, (min_v, max_v)) in enumerate(real_params.iteritems()) 
    ])
    for p in ["n_neg", "n_pos", "hidden_size"]:
        ans[p] = int(ans[p])
    return ans

def run_then_return_val_loss(num_iters, hyperparameters):
    epsilon = hyperparameters["epsilon"]
    hidden_size = hyperparameters["hidden_size"]
    lrate0 = hyperparameters["lrate0"]
    lrate1 = hyperparameters["lrate1"]
    n_neg = hyperparameters["n_neg"]
    n_pos = hyperparameters["n_pos"]
    beta = hyperparameters["beta"]

    x_values, y_values = get_toy_data(seed=2)

    input_size = x_values.shape[1]
    output_size = len(np.unique(y_values))
    batch_size = 100
    net_size = input_size + hidden_size + output_size
    sl0, sl1, sl2 = input_size, input_size+hidden_size, input_size+hidden_size+output_size

    def make_feed_forward(w):
        w[:sl0, :sl0] = 0.0
        w[sl0:sl1, sl0:sl1] = 0.0
        w[sl1:sl2, sl1:sl2] = 0.0
        w[:sl0, sl1:sl2] = 0.0
        w[sl1:sl2, :sl0] = 0.0
        return w

    def make_lrate_matrices(lrate01, lrate12):
        w_lrates = np.zeros(W.shape)
        w_lrates[:sl0, sl0:sl1] = lrate01
        w_lrates[sl0:sl1, :sl0] = lrate01
        w_lrates[sl0:sl1, sl1:sl1] = lrate12
        w_lrates[sl1:sl2, sl0:sl1] = lrate12

        b_lrates = np.zeros(bias.shape)
        # b_lrates[:sl0] = lrate01
        b_lrates[sl0:sl1] = lrate01
        b_lrates[sl1:sl2] = lrate12
        
        return w_lrates, b_lrates


    W = initialize_layer(net_size, net_size)
    W = (W + W.T)/2.0
    W = make_feed_forward(W)

    bias = np.zeros((net_size,))

    act = ClipActivation()
    model = ExpDecayHopfield(act, W, bias)
    cost = MseCost()
    n_train = (4 * x_values.shape[0]/5)/batch_size
    n_valid = (1 * x_values.shape[0]/5)/batch_size

    n_batches = x_values.shape[0]/batch_size

    u_p = np.zeros((batch_size, net_size))
    u_p_v = np.zeros((batch_size, net_size))

    w_lrates, b_lrates = make_lrate_matrices(lrate0, lrate1)

    opt = SGDOpt([w_lrates, b_lrates])

    opt.init(model.W, model.b)

    for e in xrange(num_iters):
        cost_val_stat_t, dW_stat_t, db_stat_t, acc_stat_t = 0.0, 0.0, 0.0, 0.0

        mom_acc, db_acc = np.zeros(model.W.shape), np.zeros(model.b.shape)
        
        # for index in np.random.permutation(xrange(n_train)):
        for index in xrange(n_train):
            # u = u_p[index * batch_size: (index + 1) * batch_size]
            u = u_p.copy()

            x_v = get_batch(x_values, index, batch_size=batch_size)
            y_idx = get_batch(y_values, index, batch_size=batch_size)
            y_v = one_hot(y_idx, output_size)

            u, cost_val_neg, V_neg = model.run(
                u, n_neg, x_v, y_v, cost, epsilon=epsilon, clamped=False
            )
            
            u_pos, _, V_pos = model.run(
                u, n_pos, x_v, y_v, cost, epsilon=epsilon, clamped=True, beta=beta
            )

            u[:, :input_size] = x_v
            u_pos[:, :input_size] = x_v


            dWn, dbn = model.grad(u)
            dWp, dbp = model.grad(u_pos)
            
            dW = 2.0 * (dWp - dWn) / beta / batch_size
            db = (dbp - dbn) / beta / batch_size
        
            
            model.W, model.b = opt.update((model.W, dW), (model.b, db))

            cost_val_stat_t += np.mean(cost_val_neg)
            dW_stat_t += np.mean(dW)
            db_stat_t += np.mean(db)
            acc_stat_t += np.mean(np.argmax(u[:,-output_size:],1) != y_idx)
                
            # u_p[index * batch_size: (index + 1) * batch_size] = u
            u_p = u
            # shm(u[:,input_size:], u_pos[:,input_size:], file=pj(tmp_dir, "{}_{}_u.png".format(e, index)))
                
        cost_val_stat_v, acc_stat_v = 0.0, 0.0

        for index in xrange(n_valid):
            dindex = n_train+index
            u = u_p_v.copy() #u_p[dindex * batch_size: (dindex + 1) * batch_size]

            x_v = get_batch(x_values, dindex, batch_size=batch_size)
            y_idx = get_batch(y_values, dindex, batch_size=batch_size)
            y_v = one_hot(y_idx, output_size)

            u, cost_val_neg, V_neg = model.run(
                u, n_neg, x_v, y_v, cost, epsilon=epsilon, clamped=False
            )
            
            cost_val_stat_v += np.mean(cost_val_neg)
            acc_stat_v += np.mean(np.argmax(u[:,-output_size:],1) != y_idx)
            u[:, :input_size] = x_v

            # u_p[dindex*batch_size: (dindex + 1)*batch_size] = u
            u_p_v = u

    print cost_val_stat_v/n_valid, acc_stat_v/n_valid
    return cost_val_stat_v/n_valid


max_iter = 200  # maximum iterations/epochs per configuration
eta = 3 # defines downsampling rate (default=3)
logeta = lambda x: log(x)/log(eta)
s_max = int(logeta(max_iter))  # number of unique executions of Successive Halving (minus one)
B = (s_max+1)*max_iter  # total number of iterations (without reuse) per execution of Succesive Halving (n,r)

#### Begin Finite Horizon Hyperband outlerloop. Repeat indefinetely.
for s in reversed(range(s_max+1)):
    n = int(ceil(B/max_iter/(s+1)*eta**s)) # initial number of configurations
    r = max_iter*eta**(-s) # initial number of iterations to run configurations for

    #### Begin Finite Horizon Successive Halving with (n,r)
    T = [ get_random_hyperparameter_configuration() for i in range(n) ] 
    for i in range(s+1):
        # Run each of the n_i configs for r_i iterations and keep best n_i/eta
        n_i = n*eta**(-i)
        r_i = r*eta**(i)
        val_losses = [ run_then_return_val_loss(num_iters=int(ceil(r_i)),hyperparameters=t) for t in T ]
        T = [ T[i] for i in argsort(val_losses)[0:int( n_i/eta )] ]
    #### End Finite Horizon Successive Halving with (n,r)