

import time

from util import shl, shm, shs
from datasets import get_toy_data

from tensorflow.python.ops import rnn
# import tensorflow.contrib.rnn as rnn


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
        tf.placeholder(tf.float32, (None, size)) 
        for size in cell.state_size
    ) 
    for cell in net.cells
) + (tf.placeholder(tf.float32, (None, output_size)),)



# x_list = tf.unstack(x, num_steps, 1)
# y_list = tf.unstack(y, num_steps, 1)

# modulation_list = tf.unstack(modulation, num_steps, 1)

out, finstate = rnn.dynamic_rnn(net, (x, y, modulation), initial_state=state, dtype=tf.float32)

# x_list = tf.unstack(x, num_steps, 1)
# y_list = tf.unstack(y, num_steps, 1)

# modulation_list = tf.unstack(modulation, num_steps, 1)

# out, finstate = rnn.static_rnn(net, zip(x_list, y_list, modulation_list), initial_state=state, dtype=tf.float32)


init = tf.global_variables_initializer()

####################################################################


get_zero_state = lambda: tuple(
    np.zeros((batch_size, t.get_shape().as_list()[1])) 
    for tup in state[:-1] for t in tup
) + (
    np.zeros((batch_size, output_size)),
)

input_values = poisson_samples(x_values, num_steps, c.lambda_max, c.dt)

####################################################################

sess = tf.Session()
sess.run(init)

def run(mod, l_id, r_id):
    y_v = np.tile(y_hot[l_id:r_id], num_steps).reshape(batch_size, num_steps, output_size)

    out_v, state_v = sess.run(
        (out, finstate), 
    {
        x: input_values[l_id:r_id],
        y: y_v,
        modulation: mod * np.ones((batch_size, num_steps, 1)),
        state: get_zero_state()
    })

    return out_v, state_v[:-1]

for e in xrange(100):
    start_time = time.time()
    
    train_log_loss, train_error_rate, test_log_loss, test_error_rate = 0.0, 0.0, 0.0, 0.0

    updates = []
        
    for b_id in xrange(n_train_batches):
        l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)
        
        
        out_f, sf = run(0.0, l_id, r_id)
        out_t, st = run(1.0, l_id, r_id)
        
        for sf_i, st_i, cell in zip(sf[:-1], st[:-1], net.cells[:-1]):
            Wff, bias_ff, Wfb = cell.params

            basal_state_f, basal_state_m_f, soma_state_f, soma_state_m_f, rate_m_f, apical_m_f = sf_i
            basal_state_t, basal_state_m_t, soma_state_t, soma_state_m_t, rate_m_t, apical_m_t = st_i

            alpha_f = act(apical_m_f)
            alpha_t = act(apical_m_t)

            deriv_part = - (alpha_t - alpha_f) * act.grad(soma_state_m_f)
            d_bias_ff = np.mean(deriv_part, 0)
            d_Wff = np.mean(batch_outer(basal_state_m_f, deriv_part), 0)
        
            updates += [
                (Wff, 10.0 * d_Wff),
                (bias_ff, 10.0 * d_bias_ff),
            ]
            
            
        Wff, bias_ff, _ = net.cells[-1].params

        basal_state_o_f, basal_state_m_o_f, soma_state_o_f, soma_state_m_o_f, rate_m_o_f = sf[-1]
        basal_state_o_t, basal_state_m_o_t, soma_state_o_t, soma_state_m_o_t, rate_m_o_t = st[-1]

        deriv_part = - (rate_m_o_t - rate_m_o_f) * act.grad(soma_state_m_o_f)
        d_bias_ff = np.mean(deriv_part, 0)
        d_Wff = np.mean(batch_outer(basal_state_m_o_f, deriv_part), 0)

        updates += [
            (Wff, 0.1 * d_Wff),
            (bias_ff, 0.1 * d_bias_ff),
        ]
        
        
        train_log_loss += log_loss(y_hot[l_id:r_id], rate_m_o_f/c.lambda_max)
        train_error_rate += np.mean(np.argmax(rate_m_o_f, 1) != y_values[l_id:r_id])

    sess.run(tuple(tf.assign_sub(ref, sub) for ref, sub in updates))

    for b_id in xrange(n_train_batches, n_train_batches+n_test_batches):
        l_id, r_id = (b_id*batch_size), ((b_id+1)*batch_size)
        
        out_v, state_v = run(0.0, l_id, r_id)

        rate_m_o = state_v[-1][-1]

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


l0, l1 = out_f

rate0, spike0 = l0
rate1, spike1 = l1
