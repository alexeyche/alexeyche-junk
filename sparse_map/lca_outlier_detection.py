import os

import tensorflow as tf
import pandas as pd
from functools import partial
import time
from util import *

from hist_rnn import rnn_with_hist_loop_fn
from config import Config
from env import Env

from model import LCACell, normalize_weights, exp_poisson, poisson, LCAScalarCell
from tensorflow.contrib.rnn import MultiRNNCell

import scikits.statsmodels.tsa.api as smt
from scikits.statsmodels.tsa.arima_process import arma_generate_sample

from ts_pp import white_ts, generate_ts

from util import *

import scipy.io

ds = scipy.io.loadmat("{}/datasets/outlier_detection/thyroid.mat".format(os.environ["HOME"]))
Xraw = ds["X"][:, :]
y = ds["y"][:, :]

X = Xraw.copy()
X[np.where(y)[0]] = (X[np.where(y)[0]-1] + X[np.where(y)[0]+1])/2.0


# assert X.shape[0] % 4 == 0
lrate = 0.0001
# lrate *= 100.0

epochs = 100

seed = 7
tf.set_random_seed(seed)
np.random.seed(seed)


input_size = X.shape[1]
batch_size = 1
data_size = X.shape[0]
seq_size = data_size/batch_size

layer_size = 100

c = Config()

c.weight_init_factor = 0.3
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0

c.lam = 0.05
c.adaptive_threshold = False

c.tau_m = 200.0
c.adapt = 5.0
c.act_factor = 1.0
c.adaptive = False

c.tau_fb = 10.0
c.fb_factor = 2.0
c.smooth_feedback = False


hand_grad = False

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")

act = tf.nn.relu
# act = lambda x: poisson(tf.nn.relu(x))

net = MultiRNNCell([
    LCAScalarCell(input_size, layer_size, c, act=act),
])

state = tuple(
    tuple((
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="u"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a_m"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="fb_m"),
        tf.placeholder(tf.float32, [batch_size, cell.input_size, cell.layer_size], name="dF"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size, cell.layer_size], name="dFc"),
    )) for cell in net._cells
)

get_zero_state = lambda: tuple(
    np.zeros((batch_size,) + tuple(t.get_shape().as_list()[1:]))
    for tup in state for t in tup
)


(a, u, a_m, x_hat, fb), finstate = tf.nn.dynamic_rnn(net, (input, ), initial_state=state, time_major=True)

loss = tf.nn.l2_loss(input - x_hat)

# loss = l2 (input - (f(input*W) * W.T))
# dloss = 0.5 * (input - )

dF_grad_base, dFc_grad_base = tf.gradients(loss, [net._cells[0]._params[0], net._cells[0]._params[1]])


optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)

grads_and_vars = []
if hand_grad:
    for li, s in enumerate(finstate):
        dF = s[-2]
        dFc = s[-1]
        
        grads_and_vars += [
            (tf.reduce_sum(dF, 0), net._cells[li].F),
            (tf.reduce_sum(dFc, 0), net._cells[li].Fc),
        ]
else:
    grads_and_vars = [
        (dF_grad_base, net._cells[0].F),
        (dFc_grad_base, net._cells[0].Fc)
    ]
    

apply_grads_step = tf.group(
    optimizer.apply_gradients(grads_and_vars),
    normalize_weights(net)
)



sess = tf.Session()

sess.run(tf.global_variables_initializer())


# X = svd_whiten(X)
x_v = X.reshape((seq_size, batch_size, input_size))
# x_v = (x_v - np.mean(x_v, 0))
# x_v = (x_v - np.mean(x_v, 0))/np.std(x_v, 0)

# sess.run(tf.group(*[tf.assign(cell.F, tf.nn.l2_normalize(cell.F, 0)) for cell in net._cells]))
# sess.run(tf.group(*[tf.assign(cell.Fc, tf.matmul(tf.transpose(cell.F), cell.F) - tf.eye(cell.F.get_shape()[1].value)) for cell in net._cells]))
sess.run(tf.group(*[tf.assign(cell.Fc, tf.matrix_set_diag(tf.matmul(tf.transpose(cell.F), cell.F), tf.zeros(cell.F.get_shape()[1].value))) for cell in net._cells]))

state_v = get_zero_state()
    
for e in xrange(epochs):
    start_time = time.time()

    u_v, a_v, a_m_v, x_hat_v, state_v, F_v, loss_v, dF_grad_b, dFc_grad_b, fb_v, _ = sess.run(
        (
            u, 
            a, 
            a_m, 
            x_hat, 
            finstate, 
            net._cells[0].F, 
            loss,
            dF_grad_base,
            dFc_grad_base,
            fb,
            apply_grads_step,
        ), 
        {
            input: x_v,
            state: state_v,
        }
    )
    # sess.run(tf.variables_initializer([x_hat_var]))
    
    end_time = time.time()
    
    print "Epoch {} ({}), MSE {:}".format(
        e, 
        round(end_time-start_time, 3), 
        loss_v
    )


# shl(a_v[:500, 0, :])
shm(a_v[100:500,0,:])
shl(x_v[:,0,0], x_hat_v[:,0,0])


x_hat_v = sess.run(x_hat, {input: Xraw.reshape(seq_size, batch_size, input_size), state: state_v})
error = np.sum(np.sum(np.square(x_hat_v - Xraw.reshape(seq_size, batch_size, input_size)), 1), 1)
shl(error, -y)