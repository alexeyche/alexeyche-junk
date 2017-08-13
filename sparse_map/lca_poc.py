
import os

from tensorflow.contrib.rnn import RNNCell as RNNCell
import tensorflow as tf

from util import *

from hist_rnn import rnn_with_hist_loop_fn
from config import Config
from env import Env


def norm(w):
    return np.asarray([w[:,i]/np.sqrt(np.sum(np.square(w[:,i]))) for i in xrange(w.shape[1])]).T


class LCACell(RNNCell):
    def __init__(self, input_size, layer_size, filter_len, c):
        self._layer_size = layer_size
        self._filter_len = filter_len
        self._input_size = input_size
        self._c = c
        self._params = None

    @property
    def state_size(self):
        return (self._layer_size, self._layer_size, self._layer_size)

    @property
    def output_size(self):
        return (self._layer_size, self._layer_size, self._filter_len * self._input_size)

    def _init_parameters(self):
        return (
            tf.get_variable("F", [self._filter_len * self._input_size, self._layer_size], 
                initializer=tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
            ),
        )

    def __call__(self, input, state, scope=None):
        with tf.variable_scope(scope or type(self).__name__):
            ### init
            if self._params is None:
                self._params = self._init_parameters()

            x = input[0]
            batch_size, filter_len, input_size = x.get_shape().as_list()

            u, a, dF = state
            F = self._params[0]
            
            Fc = tf.matmul(tf.transpose(F), F) - tf.eye(self._layer_size)
            
            #### logic
            
            x_flat = tf.reshape(x, (batch_size, filter_len * input_size))
            
            drive = tf.matmul(x_flat, F)
            feedback = tf.matmul(a, Fc)
            
            du = - u + drive - feedback
            new_u = u + c.epsilon * du / c.tau
            new_a = tf.nn.relu(new_u - c.lam)
            
            #### learning

            # new_dF = dF + c.grad_accum_rate * (
            #     tf.matmul(tf.transpose(x_flat), new_a) - tf.matmul(F, tf.matmul(tf.transpose(new_a), new_a))
            # )

            new_dF = dF + c.grad_accum_rate * (
                tf.matmul(tf.transpose(x_flat), new_a)
            )
            
            x_hat_flat = tf.matmul(new_a, tf.transpose(F))
            # x_hat = tf.reshape(x_hat_flat, (batch_size, filter_len, input_size))
            # print x_hat.get_shape() 
            return (new_u, new_a, x_hat_flat), (new_u, new_a, new_dF)

    @property
    def F(self):
        assert self._params is not None
        return tf.reshape(self._params[0], (self._filter_len, self._input_size, self._layer_size))

    @property
    def F_flat(self):
        assert self._params is not None
        return self._params[0]


input_size = 1
seq_size = 1000
batch_size = 2
layer_size = 25
filter_len = 50

dt = 1.0
T0, Tmax = 0.0, seq_size * dt
Tsize = int(Tmax/dt)
lrate = 0.1

c = Config()
c.lam = 1.0
c.weight_init_factor = 1.0
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0/Tmax

epochs = 1000

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")
sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)


cell = LCACell(input_size, layer_size, filter_len, c)

state = (
    tf.placeholder(tf.float32, [batch_size, layer_size], name="u"),
    tf.placeholder(tf.float32, [batch_size, layer_size], name="a"),
    tf.placeholder(tf.float32, [batch_size, filter_len*input_size, layer_size], name="dF"),
)

get_zero_state = lambda: tuple(
    np.zeros((batch_size,) + tuple(t.get_shape().as_list()[1:]))
    for t in state
)


(u_ta, a_ta, x_hat_flat_ta), finstate, _ = tf.nn.raw_rnn(
    cell, 
    rnn_with_hist_loop_fn(input, sequence_length, state, filter_len)
)

u, a, x_hat_flat = u_ta.stack(), a_ta.stack(), x_hat_flat_ta.stack()

x_hat = tf.reshape(x_hat_flat, (seq_size, batch_size, filter_len, input_size))
# x_hat = tf.reduce_mean(x_hat, 2)

sess = tf.Session()
saver = tf.train.Saver()

env = Env("lca_poc")


model_fname = env.run("model.ckpt")
if os.path.exists(model_fname):
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0
else:
    sess.run(tf.global_variables_initializer())


env.clear_pics(env.run())


T = np.linspace(T0, Tmax, Tsize)
x_v = np.asarray([
    1.0*np.sin(T/10.0),
    1.0*np.sin(20+T/10.0),
]).T.reshape(Tsize, batch_size, 1)


for e in xrange(100):
    state_v = get_zero_state()

    u_v, a_v, x_hat_v, (ue_v, ae_v, dF_v) = sess.run(
        (u, a, x_hat, finstate), 
        {
            input: x_v,
            state: state_v,
            sequence_length: np.asarray([seq_size]*batch_size)
        }
    )

    x_hat_f_v = np.zeros((seq_size, batch_size, input_size))
    for ti in xrange(x_hat_v.shape[0]):
        left_ti = max(0, ti-filter_len)
        x_hat_f_v[left_ti:ti] += np.transpose(x_hat_v[ti,:, :(ti-left_ti), :], (1, 0, 2))/filter_len


    # F_flat = sess.run(cell.F_flat)    
    # sess.run(tf.assign(cell.F_flat, norm(F_flat + lrate * np.sum(dF_v, 0))))

    sess.run(tf.assign_add(cell.F_flat, lrate * np.mean(dF_v, 0)))

    print "Epoch {}, MSE {}".format(e, np.mean(np.square(x_hat_f_v - x_v)))

shl(x_v[:,0], x_hat_f_v[:,0])