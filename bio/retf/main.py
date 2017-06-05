

import time

from util import shl, shm, shs
from datasets import get_toy_data

# from tensorflow.python.ops import rnn
import tensorflow.contrib.rnn as rnn


import tensorflow as tf
import numpy as np
from sklearn.metrics import log_loss        


from config import Config
from retf.model import *
from activation import *

def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)

def one_hot(y, y_size):
    y_oh = np.zeros((y.shape[0], y_size))
    y_oh[np.arange(y.shape[0]), y] = 1.0
    return y_oh

def batch_outer(left, right):
    return np.asarray([np.outer(left[i], right[i]) for i in xrange(left.shape[0])])


def poisson_samples(x_values, Tsize, lambda_max, dt):
    n_samples = x_values.shape[0]
    input_size = x_values.shape[1]
    
    out = np.zeros((n_samples, Tsize, input_size))
    
    for ti in xrange(Tsize):
        out[:, ti, :] = poisson(lambda_max * x_values, dt)
    
    return out

#############################################


x_values, y_values = get_toy_data()
y_uniq = np.unique(y_values)
y_hot = one_hot(y_values, len(y_uniq))

n_train = (4 * x_values.shape[0]/5)
n_test = (1 * x_values.shape[0]/5)

batch_size = 400

n_train_batches = n_train/batch_size
n_test_batches = n_test/batch_size

T = 50.0
dt = 1.0
num_steps = int(T/dt)

input_size = x_values.shape[-1]
state_size = 20
output_size = len(y_uniq)

###

c = Config()

c.batch_size = batch_size
c.lambda_max = 200.0/1000.0      # 1/ms

c.weight_init_factor = 0.1
c.dt = dt
c.tau_c = 10.0
c.tau_b = 2.0
c.tau_a = 2.0
c.tau_syn = 10.0 

c.Ee = 8.0
c.Ei = -8.0
c.alpha = c.dt/10.0

tf.set_random_seed(10)

#############################################

act = SigmoidActivation()

net = DnnNet(
    DnnCell((input_size, state_size, output_size), act=act.tf_call, is_output_cell=False, c=c),
    DnnCell((state_size, output_size, output_size), act=act.tf_call, is_output_cell=True, c=c),
)


x = tf.placeholder(tf.float32, (None, num_steps, input_size), name="x")
y = tf.placeholder(tf.float32, (None, num_steps, output_size), name="y")

modulation = tf.placeholder(tf.float32, (None, num_steps, 1))

state = tuple(
    tuple(
        tf.placeholder(tf.float32, (None,) + (size if type(size) is tuple else (size,)))
        for size in cell.state_size
    ) 
    for cell in net.cells
) + (tf.placeholder(tf.float32, (None, output_size)),)


# out, finstate = rnn.dynamic_rnn(net, (x, y, modulation), initial_state=state, dtype=tf.float32)


x_list = tf.unstack(x, num_steps, 1)
y_list = tf.unstack(y, num_steps, 1)

modulation_list = tf.unstack(modulation, num_steps, 1)

out, finstate = rnn.static_rnn(net, zip(x_list, y_list, modulation_list), initial_state=state, dtype=tf.float32)


optimizer = tf.train.AdamOptimizer(0.01)
# optimizer = tf.train.GradientDescentOptimizer(10.0)

grads_and_vars = []
for li, s in enumerate(finstate[:-1]):
    grads_and_vars += [
        (tf.reduce_mean(s[5], 0)/num_steps, net.cells[li]._params[0]),
        (tf.reduce_mean(s[6], 0)/num_steps, net.cells[li]._params[1]),
    ]

    # if li < len(net.cells)-1:
    #     grads_and_vars.append(
    #         (tf.reduce_mean(s[7], 0)/num_steps, net.cells[li]._params[2]),
    #     )
    

apply_grads_step = optimizer.apply_gradients(grads_and_vars)



init = tf.global_variables_initializer()

####################################################################


get_zero_state = lambda: tuple(
    np.zeros((batch_size,) + tuple(t.get_shape().as_list()[1:])) 
    for tup in state[:-1] for t in tup
) + (
    np.zeros((batch_size, output_size)),
)

input_values = poisson_samples(x_values, num_steps, c.lambda_max, c.dt)

####################################################################

sess = tf.Session()
sess.run(init)

def run(mod, l_id, r_id, state_value=None, apply_grads=True):
    if state_value is None:
        state_value = get_zero_state() 
    y_v = np.tile(y_hot[l_id:r_id], num_steps).reshape(batch_size, num_steps, output_size)

    outs = sess.run(
        (out, finstate) + ((apply_grads_step,) if apply_grads else tuple()), 
    {
        x: input_values[l_id:r_id],
        y: y_v,
        modulation: mod * np.ones((batch_size, num_steps, 1)),
        state: state_value
    })

    return outs[0], outs[1]

for e in xrange(100):
    start_time = time.time()
    
    train_log_loss, train_error_rate, test_log_loss, test_error_rate = 0.0, 0.0, 0.0, 0.0

    for b_id in xrange(n_train_batches):
        l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)
        
        out_f, sf = run(0.0, l_id, r_id)
        out_t, st = run(1.0, l_id, r_id, state_value=sf)
            
        rate_m_o_f = sf[-2][4]
        
        train_log_loss += log_loss(y_hot[l_id:r_id], rate_m_o_f/c.lambda_max)
        train_error_rate += np.mean(np.argmax(rate_m_o_f, 1) != y_values[l_id:r_id])

    for b_id in xrange(n_train_batches, n_train_batches+n_test_batches):
        l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)
        
        out_v, state_v = run(0.0, l_id, r_id, apply_grads=False)

        rate_m_o = state_v[-2][4]

        test_log_loss += log_loss(y_hot[l_id:r_id], rate_m_o/c.lambda_max)
        test_error_rate += np.mean(np.argmax(rate_m_o, 1) != y_values[l_id:r_id])


    duration = time.time() - start_time
    
    print "Epoch {} ({:.2f}s), train ll {:.3f}, train error {:.3f}, test ll {:.3f}, test error {:.3f}".format(
        e, 
        duration, 
        train_log_loss/n_train_batches, 
        train_error_rate/n_train_batches, 
        test_log_loss/n_test_batches,
        test_error_rate/n_test_batches, 
    )


# l0, l1 = out_f

# rate0, spike0 = l0
# rate1, spike1 = l1
