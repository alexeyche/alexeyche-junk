
import time

from util import shl, shm, shs

from tensorflow.python.ops import rnn
# import tensorflow.contrib.rnn as rnn

import tensorflow as tf
import numpy as np
from sklearn.metrics import log_loss

from config import Config
from retf.model import *

def poisson(x, dt):
    return (dt * x > np.random.random(x.shape)).astype(np.int8)


hidden_size = 50

batch_size = 1

T0 = 0.0
T = 100.0
dt = 1.0
num_steps = int(T/dt)

###

c = Config()

c.batch_size = batch_size
c.lambda_max = 500.0/1000.0      # 1/ms

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

### data

input_size = 300
output_size = 20    
x_values = np.zeros((num_steps, output_size))
for ti in xrange(0, num_steps, 5):
    x_values[ti, ti % output_size] = 1.0


# input_values = np.zeros((num_steps, input_size))
# for ti in xrange(0, num_steps, 5):
#     input_values[ti, input_size - (ti % input_size)-1] = 1.0

# input_values = x_values.copy()
input_values = poisson(0.02*np.random.random((num_steps, input_size)), c.dt)


###

net = DnnNet(
    DnnCell((input_size, output_size, output_size), act=tf.sigmoid, is_output_cell=True, c=c),
)

x = tf.placeholder(tf.float32, (None, num_steps, input_size), name="x")
y = tf.placeholder(tf.float32, (None, num_steps, output_size), name="y")

modulation = tf.placeholder(tf.float32, (None, num_steps, 1))

state = tuple(
    tuple(
        tf.placeholder(tf.float32, (None,) + (size if isinstance(size, tuple) else (size,)))
        for size in cell.state_size
    ) 
    for cell in net.cells
) + (tf.placeholder(tf.float32, (None, output_size)),)


out, finstate = rnn.dynamic_rnn(net, (x, y, modulation), initial_state=state, dtype=tf.float32)


# x_list = tf.unstack(x, num_steps, 1)
# y_list = tf.unstack(y, num_steps, 1)

# modulation_list = tf.unstack(modulation, num_steps, 1)

# out, finstate = rnn.static_rnn(net, zip(x_list, y_list, modulation_list), initial_state=state, dtype=tf.float32)

optimizer = tf.train.AdamOptimizer(0.001)
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

def smooth(signal, sigma=0.005, filter_size=50):
    lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
    return np.convolve(lf_filter, signal, mode="same")



sess = tf.Session()
sess.run(init)

theta = 3.0 

Tvec = np.linspace(T0, T, num_steps)

rhythm = np.sin(2.0 * np.pi * Tvec * theta/T)/2.0 + 0.5
# rhythm = np.zeros((num_steps,))

rhythm_b = np.expand_dims(np.expand_dims(rhythm, 0), 2)


input_values_b = np.expand_dims(input_values, 0)

x_values_sm = np.asarray([smooth(x_values[:,ni]) for ni in xrange(output_size)]).T

x_values_sm_b = np.expand_dims(x_values_sm, 0)


for e in xrange(500):
    start_time = time.time()

    out_v, state_v, _ = sess.run(
        (out, finstate, apply_grads_step,),
    {
        x: input_values_b,
        y: x_values_sm_b,
        modulation: rhythm_b,
        state: get_zero_state() 
    })


    out_v_test, state_v_test = sess.run(
        (out, finstate,),
    {
        x: input_values_b,
        y: x_values_sm_b,
        modulation: np.zeros((batch_size, num_steps, 1)),
        state: get_zero_state() 
    })

    duration = time.time() - start_time

    error = np.sum(np.square(out_v_test[0][0]/c.lambda_max - x_values_sm))

    print "Epoch {} ({:.2f}s), train error {:.3f}".format(
        e, 
        duration, 
        error
    )
