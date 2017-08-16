
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
                initializer=tf.random_uniform_initializer(-0.5, 0.5) #tf.uniform_unit_scaling_initializer(factor=c.weight_init_factor)
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
            # ops.append(tf.assign(F, tf.nn.l2_normalize(F, 0)))
    return tf.group(*ops)


lrate = 0.1
epochs = 50

tf.set_random_seed(3)
np.random.seed(3)

input_size = 1
seq_size = 350
batch_size = 2
layer_size = 25
filter_len = 50

dt = 1.0

c = Config()
c.lam = 0.2
c.weight_init_factor = 1.0
c.epsilon = 1.0
c.tau = 5.0
c.grad_accum_rate = 1.0/seq_size
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


T = np.linspace(filter_len*dt, seq_size*dt - 2*filter_len, int((seq_size-2*filter_len)/dt))
# x_v = np.asarray([
#     1.0*np.sin(T/10.0),
# ]).T.reshape(Tsize, batch_size, 1)


d0 = np.pad(np.sin(T/10.0), (filter_len, filter_len), 'constant')
d1 = np.pad(np.sin(T/10.0 + 10.0), (filter_len, filter_len), 'constant')

x_v = np.asarray([d0, d1]).T.reshape(d0.shape[0], batch_size, 1)

# x_v = x_v/np.sqrt(np.mean(np.square(x_v), 0))

l2_norm = lambda x: np.sqrt(np.sum(np.square(x), 0))
x_v_n = np.asarray([l2_norm(x_v[(ti-filter_len):ti]) for ti in xrange(filter_len, x_v.shape[0])])
x_v[filter_len:-filter_len] = x_v[filter_len:-filter_len]/x_v_n[(filter_len/2):-(filter_len/2)]


sess.run(tf.assign(net._cells[0].F_flat, tf.nn.l2_normalize(net._cells[0].F_flat, 0)))

for e in xrange(100):
    state_v = get_zero_state()
    
    u_v, a_v, x_hat_v, finstate_v, F_v, _ = sess.run(
        (u, a, x_hat, finstate, net._cells[0].F_flat, apply_grads_step), 
        {
            input: x_v,
            state: state_v,
            sequence_length: np.asarray([seq_size]*batch_size)
        }
    )
    
    x_hat_f_v = np.zeros((seq_size, batch_size, input_size))
    for ti in xrange(x_hat_v.shape[0]):
        left_ti = max(0, ti-filter_len)
        x_hat_f_v[left_ti:ti] += np.transpose(x_hat_v[ti,:, :(ti-left_ti), :], (1, 0, 2))

    # x_hat_f_v = x_hat_f_v/35.0
    print "Epoch {}, MSE {}".format(e, np.mean(np.square(x_hat_f_v[filter_len:-filter_len] - x_v[filter_len:-filter_len])))

shl(x_hat_f_v, x_v, show=False)
shm(a_v[:,0,:], a_v[:,1,:])
