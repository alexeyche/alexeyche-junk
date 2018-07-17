

from poc.datasets import *
from poc.util import *

import tensorflow as tf

def sigmoid_deriv(x):
    v = tf.sigmoid(x)
    return v * (1.0 - v)

def ltd(a_ff, a_fb):
    ltd = tf.zeros(tf.shape(a_ff))
    ltd = tf.where(a_fb < 1e-10, a_ff, ltd)
    return ltd

def relu_deriv(x):
    a = tf.nn.relu(x)
    da = tf.where(a > 0.0, tf.ones(tf.shape(a)), tf.zeros(tf.shape(a)))
    return da

np.random.seed(12)
tf.set_random_seed(12)

ds = XorDatasetSmall()

(_, input_size), (_, output_size) = ds.train_shape


x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")


weight_factor = 1.0
layer_size = 50

W0v = 0.1 * (np.random.random((input_size, layer_size)) - 0.5).astype(np.float32)
W1v = 0.1 * (np.random.random((layer_size, output_size)) - 0.5).astype(np.float32)
b0v = 0.1 * (np.random.random((layer_size,)) - 0.5).astype(np.float32)
b1v = 0.1 * (np.random.random((output_size,)) - 0.5).astype(np.float32)

W0 = tf.Variable(W0v)
W1 = tf.Variable(W1v)
b0 = tf.Variable(b0v)
b1 = tf.Variable(b1v)

##

u0 = tf.matmul(x, W0) + b0
a0 = tf.nn.relu(u0)

u1 = tf.matmul(a0, W1) + b1
# a1 = tf.sigmoid(u1)
a1 = tf.nn.relu(u1)

a0_mp = tf.nn.relu(tf.matmul(y, tf.transpose(W1)))



loss = tf.square(a1 - y) / 2.0

du1 = (y - a1) * relu_deriv(u1)
du0 = tf.matmul(du1, tf.transpose(W1)) * relu_deriv(u0)
dW0 = tf.matmul(tf.transpose(x), du0 * relu_deriv(u0))


du0_mp = (a0_mp - ltd(a0, a0_mp)) * relu_deriv(u0)

gr = tf.gradients(loss, [u0, u1])
gr = [-g for g in gr]

dW0r = tf.gradients(loss, [W0])[0]

xv, yv = ds.next_train_batch()

sess = tf.Session()
sess.run(tf.global_variables_initializer())

gv, grv, gr0mp, dW0v, dW0rv, u, a, amp, relu_deriv_v = sess.run([
    [du0, du1],
    gr,
    [du0_mp, du1],
    dW0,
    dW0r,
    [u0, u1],
    [a0, a1],
    [a0_mp, a1],
    relu_deriv(u0)
], {x: xv, y: yv})
