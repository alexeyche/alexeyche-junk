
import os

import tensorflow as tf

from util import *

from hist_rnn import rnn_with_hist_loop_fn
from config import Config
from env import Env

from model import LCACell, normalize_weights
from tensorflow.contrib.rnn import MultiRNNCell

import scipy.io

import numpy as np

def svd_whiten(X):

    U, s, Vt = np.linalg.svd(X, full_matrices=False)

    # U and Vt are the singular matrices, and s contains the singular values.
    # Since the rows of both U and Vt are orthonormal vectors, then U * Vt
    # will be white
    X_white = np.dot(U, Vt)

    return X_white

ds = scipy.io.loadmat("{}/datasets/outlier_detection/thyroid.mat".format(os.environ["HOME"]))

X = ds["X"]
y = ds["y"]

assert X.shape[0] % 4 == 0

lrate = 0.001
epochs = 50

tf.set_random_seed(3)
np.random.seed(3)

input_size = X.shape[1]
data_size = X.shape[0]
batch_size = 1
seq_size = data_size/batch_size
layer_size = 100
filter_len = 100

dt = 1.0

c = Config()
c.lam = 0.01
c.weight_init_factor = 1.0
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0/seq_size
c.simple_hebb = True
c.tau_m = 100.0

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")
sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)

net = MultiRNNCell([
    LCACell(input_size, layer_size, filter_len, c),
])

state = tuple(
    tuple((
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="u"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a_m"),
        tf.placeholder(tf.float32, [batch_size, cell.filter_len*cell.input_size, cell.layer_size], name="dF"),
    )) for cell in net._cells
)

get_zero_state = lambda: tuple(
    np.zeros((batch_size,) + tuple(t.get_shape().as_list()[1:]))
    for tup in state for t in tup
)


(u_ta, a_ta, x_hat_flat_ta), finstate, _ = tf.nn.raw_rnn(
    net, 
    rnn_with_hist_loop_fn(input, sequence_length, state, filter_len)
)


u, a, x_hat_flat = u_ta.stack(), a_ta.stack(), x_hat_flat_ta.stack()

x_hat = tf.reshape(x_hat_flat, (seq_size, batch_size, filter_len, input_size))


optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)

grads_and_vars = []
for li, s in enumerate(finstate):
    dF = s[-1]
    grads_and_vars += [
        (-tf.reduce_mean(dF, 0), net._cells[li].F_flat),
    ]


apply_grads_step = tf.group(
    optimizer.apply_gradients(grads_and_vars),
    normalize_weights(net)
)



sess = tf.Session()
saver = tf.train.Saver()

env = Env("lca_od")


model_fname = env.run("model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())


env.clear_pics(env.run())


l2_norm = lambda x: np.sqrt(np.maximum(np.sum(np.square(x)), 1e-10))

X = (X - np.mean(X, 0))/np.std(X,0)
X = svd_whiten(X)

# for ti in xrange(0, data_size):
#     X[max(ti-filter_len, 0):ti] = X[max(ti-filter_len, 0):ti]/l2_norm(X[max(ti-filter_len, 0):ti])


# X_norm = np.asarray([l2_norm(X[(ti-filter_len):ti]) for ti in xrange(filter_len, X.shape[0])])
# X[filter_len:-filter_len] = X[filter_len:-filter_len]/X_norm[(filter_len/2):-(filter_len/2)]


F_v0 = sess.run(tf.assign(net._cells[0].F_flat, tf.nn.l2_normalize(net._cells[0].F_flat, 0)))


X = X.reshape((data_size, 1, input_size))

for e in xrange(500):
    state_v = get_zero_state()

    u_v, a_v, X_hat_raw, finstate_v, F_v, _ = sess.run(
        (u, a, x_hat, finstate, net._cells[0].F_flat, apply_grads_step), 
        {
            input: X,
            state: state_v,
            sequence_length: np.asarray([seq_size]*batch_size)
        }
    )
    
    X_hat = np.zeros((seq_size, batch_size, input_size))
    for ti in xrange(X_hat_raw.shape[0]):
        left_ti = max(0, ti-filter_len)
        X_hat[left_ti:ti] += np.transpose(X_hat_raw[ti,:, :(ti-left_ti), :], (1, 0, 2))

    # x_hat_f_v = x_hat_f_v/35.0
    print "Epoch {}, MSE {}".format(e, np.mean(np.square(X_hat[filter_len:-filter_len] - X[filter_len:-filter_len])))

# F_v = F_v.reshape(filter_len, input_size, layer_size)
# F_v0 = F_v0.reshape(filter_len, input_size, layer_size)

# shl(x_hat_f_v, x_v, show=False)
# shm(a_v[:,0,:], a_v[:,1,:])
