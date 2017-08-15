
import os

from tensorflow.contrib.rnn import RNNCell as RNNCell
from tensorflow.contrib.rnn import MultiRNNCell
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
    def layer_size(self):
        return self._layer_size
    
    @property
    def filter_len(self):
        return self._filter_len
    
    @property
    def input_size(self):
        return self._input_size

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

            x_flat = tf.reshape(x, (batch_size, filter_len * input_size))
            
            #### logic
                        
            du = - u + tf.matmul(x_flat, F) - tf.matmul(a, Fc)
            new_u = u + c.epsilon * du / c.tau
            new_a = tf.nn.relu(new_u - c.lam)
            
            #### learning
            if c.simple_hebb:
                new_dF = dF + c.grad_accum_rate * (
                    tf.matmul(tf.transpose(x_flat), new_a)
                )
            else:
                new_dF = dF + c.grad_accum_rate * (
                    tf.matmul(tf.transpose(x_flat), new_a) 
                    - tf.matmul(F, tf.matmul(tf.transpose(new_a), new_a))
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


def normalize_weights(net):
    ops = []
    for cell in net._cells:
        F = cell._params[0]
        if cell._c.simple_hebb:
            ops.append(tf.assign(F, tf.nn.l2_normalize(F, 0)))
        else:
            ops.append(tf.identity(F))
    return tf.group(*ops)


lrate = 0.1
epochs = 100

tf.set_random_seed(1)
np.random.seed(1)

input_size = 1
seq_size = 250
batch_size = 2
layer_size = 25
filter_len = 50

dt = 1.0
T0, Tmax = 0.0, seq_size * dt
Tsize = int(Tmax/dt)

c = Config()
c.lam = 1.0
c.weight_init_factor = 1.0
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0/Tmax
c.simple_hebb = True


input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, input_size), name="Input")
sequence_length = tf.placeholder(shape=(batch_size,), dtype=tf.int32)

net = MultiRNNCell([
    LCACell(input_size, layer_size, filter_len, c),
])

state = tuple(
    tuple((
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="u"),
        tf.placeholder(tf.float32, [batch_size, cell.layer_size], name="a"),
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
# x_v = np.asarray([
#     1.0*np.sin(T/10.0),
# ]).T.reshape(Tsize, batch_size, 1)

x_v = np.asarray([
    1.0*np.sin(T/10.0),
    1.0*np.sin(20+T/10.0),
]).T.reshape(Tsize, batch_size, 1)

# for ti in xrange(filter_len, x_v.shape[0]):
#     x_v_part = x_v[(ti-filter_len):ti].copy()
#     x_v[(ti-filter_len):ti] = x_v_part/np.sqrt(np.sum(np.square(x_v_part), 0) + 1e-08)

for e in xrange(epochs):
    state_v = get_zero_state()

    u_v, a_v, x_hat_v, finstate_v, _, F_v = sess.run(
        (u, a, x_hat, finstate, apply_grads_step, net._cells[0].F_flat), 
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


    print "Epoch {}, MSE {}".format(e, np.mean(np.square(x_hat_f_v[filter_len:-filter_len] - x_v[filter_len:-filter_len])))

shl(x_v[filter_len:-filter_len,0], x_hat_f_v[filter_len:-filter_len,0], show=False)
shm(a_v[:,0,:])
