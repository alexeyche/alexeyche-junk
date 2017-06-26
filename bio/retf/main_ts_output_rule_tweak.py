
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

def smooth(signal, sigma=0.005, filter_size=50):
    lf_filter = np.exp(-np.square(0.5-np.linspace(0.0, 1.0, filter_size))/sigma)
    return np.convolve(lf_filter, signal, mode="same")

def square_window(Tsize, w):
    rhythm = np.zeros(Tsize)
    sign = 1.0
    for ti in xrange(Tsize):
        if ti % w == 0 and ti > 0:
            if sign == 1.0:
                sign = 0.0
            else:
                sign = 1.0
        rhythm[ti] = sign
    return rhythm

def act_grad(x):
    v = 1.0/(1.0 + np.exp(-x))
    return v * (1.0 - v)

def batch_outer(left, right):
    return np.asarray([np.outer(left[i], right[i]) for i in xrange(left.shape[0])])

batch_size = 1

T0 = 0.0
T = 100.0
dt = 1.0
num_steps = int(T/dt)
Tvec = np.linspace(T0, T, num_steps)

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
c.alpha = c.dt/5.0
c.lrate = 0.01

## ts related

c.theta = 0.001
c.per_epoch_shift = 3


tf.set_random_seed(10)
np.random.seed(10)

### data

input_size = 100
hidden_size = 50
output_size = 1


input_values = np.zeros((num_steps, input_size))
x_values = np.zeros((num_steps, output_size))


# input_values = poisson(0.02*np.random.random((num_steps, input_size)), c.dt)

for ti in xrange(0, num_steps, 5):
    input_values[ti, input_size - (ti % input_size)-1] = 1.0



x_values[num_steps/2,0] = 1.0

# for ti in xrange(0, num_steps, 5):
#     x_values[ti, ti % output_size] = 1.0




###

net = DnnNet(
    # DnnCell((input_size, hidden_size, output_size), is_output_cell=False, c=c),
    DnnCell((input_size, output_size, output_size), is_output_cell=True, c=c),
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

optimizer = tf.train.AdamOptimizer(c.lrate)
# optimizer = tf.train.GradientDescentOptimizer(1.0)

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



sess = tf.Session()
sess.run(init)


input_values_b = np.expand_dims(input_values, 0)
x_values_sm = np.asarray([smooth(x_values[:,ni]) for ni in xrange(output_size)]).T
x_values_sm_b = np.expand_dims(x_values_sm, 0)

for i in xrange(1):
    dw_grads, db_grads, r = [], [], []
    
    out_v_t, state_v_t = sess.run(
        (out, finstate),
    {
        x: input_values_b,
        y: x_values_sm_b,
        modulation: np.ones((batch_size, num_steps,1)),
        state: get_zero_state() 
    })

    out_v_f, state_v_f = sess.run(
        (out, finstate),
    {
        x: input_values_b,
        y: x_values_sm_b,
        modulation: np.zeros((batch_size, num_steps,1)),
        state: get_zero_state() 
    })

    dbias_bl = np.squeeze(-(out_v_t[-1][0] - out_v_f[-1][0]))
    
    dw_bl = np.mean(np.squeeze(batch_outer(np.squeeze(out_v_t[-1][2]), dbias_bl)), 0)

    for e in xrange(50):
        # rhythm = np.sin(2.0 * np.pi * Tvec * c.theta/T + e * c.per_epoch_shift/(2.0*np.pi))
        # rhythm = (rhythm + 1.0)/2.0
        rhythm = square_window(num_steps + 50, 5)[(e % 50):((e % 50 ) + num_steps)]    

        rhythm_b = np.expand_dims(np.expand_dims(rhythm, 0), 2)
        
        start_time = time.time()


        
        out_v, state_v = sess.run(
            (out, finstate),
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

        error = np.sum(np.square(out_v_test[-1][0]/c.lambda_max - x_values_sm_b))

        dw_grads.append(state_v[0][5])
        db_grads.append(state_v[0][6])
        r.append(rhythm)

    print "Epoch {} ({:.2f}s), train error {:.3f}".format(
        i, 
        duration, 
        error
    )

    r = np.asarray(r)
    dw_grads = np.asarray(dw_grads)
    db_grads = np.asarray(db_grads)
    dw_grads_m = np.mean(dw_grads, 0)
    
    # dw_grads_m = 2.0*dw_bl
    # db_grads = 2.0*dbias_bl
    
    sess.run(tf.assign_sub(net.cells[-1].params[0], 10.0*dw_grads_m.reshape(input_size, output_size)))
    sess.run(tf.assign_sub(net.cells[-1].params[1], 10.0*np.mean(db_grads).reshape(1)))

