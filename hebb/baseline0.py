
import tensorflow as tf
import os
import sys
import numpy as np
from functools import partial
from os.path import join as pj
from sys import stdout

sys.path.insert(0, "/home/alexeyche/prog/alexeyche-junk/cns/tf_dnn")
from util import xavier_init, xavier_vec_init, shl, shm
sys.path.insert(0, "/home/alexeyche/distr/Towards-a-Biologically-Plausible-Backprop")
from data_api import get_data

from hopfield_utils import symmetric_feedforward_weights
from energy import ExpDecayEnergy
from la_utils import batch_inner

tmp_dir = pj(os.environ["HOME"], "hop")


np.random.seed(5)
tf.set_random_seed(5)

weight_factor = 0.1
init = lambda shape, dtype, partition_info: xavier_init(shape[0], shape[1], const = weight_factor)
bias_init = lambda shape, dtype, partition_info: np.zeros((shape[0],))

input_size = 2
hidden_size = 100
output_size = 2

batch_size = 100

net_size = input_size + hidden_size + output_size

epsilon = 1.0
beta = 1.0
epochs = 30
steps_num = 20

[os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f.endswith(".png")]

act = lambda x: tf.clip_by_value(x, 0.0, 1.0)
# act = lambda x: x
# act = lambda x: tf.nn.sigmoid(x)4.*x-2.)

x = tf.placeholder(tf.float32, [None, input_size], name="x")
y = tf.placeholder(tf.float32, [None, output_size], name="y")
u = tf.placeholder(tf.float32, [None, net_size], name="u")
y_cl = tf.placeholder(tf.int64, [None], name="y_cl")

cost = lambda xl: tf.reduce_sum(tf.square(y - tf.slice(xl, [0, net_size - output_size], [-1, output_size])))
clamp_input = lambda ul: tf.concat_v2([x, tf.slice(ul, [0, input_size], [-1, net_size - input_size])], 1)

W0 = tf.get_variable("W0", [input_size, hidden_size], initializer = init, dtype=tf.float32)
b0 = tf.get_variable("b0", [hidden_size], initializer = bias_init, dtype=tf.float32)

W1 = tf.get_variable("W1", [hidden_size, output_size], initializer = init, dtype=tf.float32)
b1 = tf.get_variable("b1", [output_size], initializer = bias_init, dtype=tf.float32)

biases = tf.expand_dims(tf.concat_v2([tf.zeros(input_size), b0, b1], 0), 1)

W = symmetric_feedforward_weights([W0, W1])

e_calcer = ExpDecayEnergy(W, biases)

def run_network(u_start, free, beta, epsilon, denergy_stop=1e-07):
    def run_network_callback(step_num, denergy, E_prev, F_prev, u_prev):
        V = act(u_prev)
        E = tf.reduce_sum(e_calcer(V))

        if free:
            F = E
        else:
            F = E + beta * cost(V)

        dEdu = tf.gradients([F], u_prev)[0]
        
        u_new = u_prev - epsilon * dEdu
        u_new = clamp_input(u_new)
        
        # u_new = tf.Print(u_new, [step_num, F, F_prev-F], message="{} ".format(("Free run" if free else "Clamp run")))
        
        return step_num+1, F_prev-F, E, F, u_new


    step_num = tf.constant(0, dtype=tf.int32)
    energy = tf.constant(1000, dtype=tf.float32)
    total_energy = tf.constant(1000, dtype=tf.float32)
    denergy = tf.constant(1000, dtype=tf.float32)

    _, _, energy_final, total_energy_final, u_final = tf.while_loop(
        body=lambda *args: run_network_callback(*args),
        cond=lambda step_num, denergy, *_: tf.logical_and(tf.less(step_num, steps_num), tf.greater(denergy, denergy_stop)),
        loop_vars=[step_num, denergy, energy, total_energy, u_start]
    )

    return u_final, energy_final, total_energy_final


u_free, energy_free, _ = run_network(u, free=True, beta=beta, epsilon=epsilon)

u_clamp, energy_clamp, total_energy_clamp = run_network(u, free=False, beta=beta, epsilon=epsilon)

dE = (energy_clamp - energy_free)/beta

opt = tf.train.GradientDescentOptimizer(0.001)
# opt = tf.train.AdamOptimizer(0.0001)

tvars = tf.trainable_variables()
grads = tf.gradients(dE, tvars)
train_step = opt.apply_gradients(zip(grads, tvars))


p_final = act(u_free)
cost_value = cost(p_final)

p_out = tf.slice(p_final, [0, net_size-output_size], [-1, output_size])

pred_error = tf.reduce_mean(
    tf.cast(
        tf.not_equal(tf.argmax(p_out, 1), y_cl), 
        tf.float32
    )
)

x_values, y_values = get_data()

def clamp_input_np(x):
    x[:, :input_size] = x_v
    return x

u_v = np.zeros((batch_size, net_size))

sess = tf.Session()
sess.run(tf.global_variables_initializer())

n_batches_train = (4 * x_values.shape[0]/5) / batch_size
n_batches_valid = (1 * x_values.shape[0]/5) / batch_size

for epoch in xrange(20):

    F0_acc, F1_acc, cost_value_acc, pred_error_acc = [], [], [], []
    
    for index in xrange(n_batches_train):
        x_v = np.asarray(x_values[index * batch_size: (index + 1) * batch_size])
        y_val = y_values[index * batch_size: (index + 1) * batch_size]
        
        y_v = np.zeros((batch_size, output_size))
        for b_id, y_id in enumerate(y_val):
        	y_v[b_id, y_id] = 1.0
        
        u0, F0, u1, F1, dE_v, p_v, cost_value_v, grads_v, _, p0_v, p1_v, pred_error_v = sess.run(        
            [
                u_free, 
                energy_free, 
                u_clamp, 
                energy_clamp, 
                dE,
                p_final,
                cost_value,
                grads,
                train_step,
                act(u_free),
                act(u_clamp),
                pred_error
            ], 
            feed_dict={u: u_v, y: y_v, x: x_v, y_cl: y_val}
        )
        
        # shm(u0, u1, file=pj(tmp_dir, "{}_{}_act.png".format(epoch, index)))
        
        F0_acc.append(F0)
        F1_acc.append(F1)
        cost_value_acc.append(cost_value_v)
        pred_error_acc.append(pred_error_v)
        
        u_v = u0

    F0_acc = np.asarray(F0_acc)
    F1_acc = np.asarray(F1_acc)
    cost_value_acc = np.asarray(cost_value_acc)
    pred_error_acc = np.asarray(pred_error_acc)

    print "Epoch {}, F0 {}, F1 {}, cost {}, error {}".format(
        epoch, 
        np.mean(F0_acc), 
        np.mean(F1_acc), 
        np.mean(cost_value_acc),
        np.mean(pred_error_acc),
    )


