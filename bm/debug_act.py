
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


def get_batch(d, idx, batch_size):
    return d[idx*batch_size:(idx+1)*batch_size]

def one_hot(y, y_size):
    y_oh = np.zeros((y.shape[0], y_size))
    y_oh[np.arange(y.shape[0]), y] = 1.0
    return y_oh

x_values, y_values = get_toy_data(seed=2)


tmp_dir = "/home/alexeyche/bm"
# [ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]
#
np.random.seed(8)

input_size = x_values.shape[1]
hidden_size = 100
output_size = len(np.unique(y_values))
batch_size = 100
epsilon = 1.0
beta = 1.0
n_neg = 20
n_pos = 4

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


w_lrates, b_lrates = make_lrate_matrices(0.05, 0.01)


act = ExpActivation()

model = ExpDecayHopfield(act, W, bias)
cost = MseCost()



n_train = (4 * x_values.shape[0]/5)/batch_size
n_valid = (1 * x_values.shape[0]/5)/batch_size

# shs(x_values, labels=y_values)
n_batches = x_values.shape[0]/batch_size


# u_p = np.zeros((x_values.shape[0], net_size))

u = np.zeros((batch_size, net_size))

index = 0

x_v = get_batch(x_values, index, batch_size=batch_size)
y_idx = get_batch(y_values, index, batch_size=batch_size)
y_v = one_hot(y_idx, output_size)

u_neg, cost_val_neg, V_neg = model.run(
    u, n_neg, x_v, y_v, cost, epsilon=epsilon, clamped=False
)

u_pos, _, V_pos = model.run(
    u, n_pos, x_v, y_v, cost, epsilon=epsilon, clamped=True, beta=beta
)


