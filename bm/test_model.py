
import sys
import os

from util import shl, shm, shs
import numpy as np
from datasets import get_toy_data
from os.path import join as pj

from models import Hopfield, ExpDecayHopfield
from activation import ClipActivation
from cost import MseCost
from models import initialize_layer

def get_batch(d, idx, batch_size):
    return d[idx*batch_size:(idx+1)*batch_size]

def one_hot(y, y_size):
    y_oh = np.zeros((y.shape[0], y_size))
    y_oh[np.arange(y.shape[0]), y] = 1.0
    return y_oh


def make_lrate_matrices(W, bias, sl0, sl1, sl2, lrate01, lrate12):
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

def get_batch(d, idx, batch_size):
    return d[idx*batch_size:(idx+1)*batch_size]

def one_hot(y, y_size):
    y_oh = np.zeros((y.shape[0], y_size))
    y_oh[np.arange(y.shape[0]), y] = 1.0
    return y_oh

def make_feed_forward(w, sl0, sl1, sl2):
    w[:sl0, :sl0] = 0.0
    w[sl0:sl1, sl0:sl1] = 0.0
    w[sl1:sl2, sl1:sl2] = 0.0
    w[:sl0, sl1:sl2] = 0.0
    w[sl1:sl2, :sl0] = 0.0
    return w


def run(W, p_init):
    tmp_dir = "/home/alexeyche/bm"
    # [ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]
    #
    np.random.seed(5)

    input_size = 2
    hidden_size = 100
    output_size = 2
    batch_size = 50
    epsilon = 1.0
    beta = 1.0
    n_neg = 20
    n_pos = 4


    net_size = input_size + hidden_size + output_size
    sl0, sl1, sl2 = input_size, input_size+hidden_size, input_size+hidden_size+output_size

    # W = (W + W.T)/2.0

    bias = np.zeros((net_size,))

    
    def make_lrate_matrices(lrate01, lrate12):
        w_lrates = np.zeros(W.shape)
        w_lrates[:sl0, sl0:sl1] = lrate01
        w_lrates[sl0:sl1, :sl0] = lrate01
        w_lrates[sl0:sl1, sl1:sl2] = lrate12
        w_lrates[sl1:sl2, sl0:sl1] = lrate12

        b_lrates = np.zeros(bias.shape)
        b_lrates[sl0:sl1] = lrate01
        b_lrates[sl1:sl2] = lrate12
        
        return w_lrates, b_lrates

    def safe_params(W, bias):
        import cPickle
        
        biases_values  = [
            bias[:sl0], bias[sl0:sl1], bias[sl1:sl2]
        ]
        weights_values = [
            W[:sl0,sl0:sl1], W[sl0:sl1, sl1:sl2]
        ]
        to_dump = (
            biases_values, 
            weights_values, 
            {"hidden_sizes": [hidden_size], "batch_size": batch_size}, 
            {"training error": [], "validation error": []}
        )
        f = file("/home/alexeyche/distr/Towards-a-Biologically-Plausible-Backprop/net1.save", 'wb')
        cPickle.dump(to_dump, f, protocol=cPickle.HIGHEST_PROTOCOL)
        f.close()


    # safe_params(W, bias)


    w_lrates, b_lrates = make_lrate_matrices(0.05, 0.01)

    act = ClipActivation()
    model = ExpDecayHopfield(act, W, bias)
    cost = MseCost()


    x_values, y_values = get_toy_data(seed=2)
    # shs(x_values, labels=y_values)
    n_batches = x_values.shape[0]/batch_size

    u = p_init #np.zeros((batch_size, net_size))

    index = 0

    x_v = get_batch(x_values, index, batch_size=batch_size)
    y_idx = get_batch(y_values, index, batch_size=batch_size)
    y_v = one_hot(y_idx, 2)

    
    x_t, y_t = x_v, y_v
    output_size = y_t.shape[-1]
    input_size = x_t.shape[-1]

    x = u.copy()
    print "W01: ", np.mean(W[:sl0, sl0:sl1])
    print "W12: ", np.mean(W[sl0:sl1, sl1:sl2])
    dx_acc, x_acc = [], []
    
    x[:, :input_size] = x_t

    dx = model.dynamics(x)
    dx = - dx

    dx_acc.append(dx)
    x_acc.append(x)

    u, cost_val_neg, V_neg = model.run(
        u, n_neg, x_v, y_v, cost, epsilon=epsilon, clamped=False
    )

    u_pos, cost_val_pos, V_pos = model.run(
        u, n_pos, x_v, y_v, cost, epsilon=epsilon, clamped=True, beta=beta
    )

    u[:, :input_size] = x_t
    u_pos[:, :input_size] = x_t

    dWn, dbn = model.grad(u)
    dWp, dbp = model.grad(u_pos)
    
    dW = 2.0 * (dWp - dWn) / beta / batch_size
    db = (dbp - dbn) / beta / batch_size
    
        
    return V_neg, V_pos, dW, db
    