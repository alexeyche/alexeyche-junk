
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

def initialize_layer(n_in, n_out):
    rng = np.random.RandomState()
    W_values = np.asarray(
        rng.uniform(
            low=-np.sqrt(6. / (n_in + n_out)),
            high=np.sqrt(6. / (n_in + n_out)),
            size=(n_in, n_out)
        ),
        dtype=np.float32
    )
    return W_values


np.random.seed(5)
tf.set_random_seed(5)

weight_factor = 1.0
init = lambda shape, dtype, partition_info: weight_factor * initialize_layer(shape[0], shape[1])
bias_init = lambda shape, dtype, partition_info: np.zeros((shape[0],))

input_size = 2
hidden_size = 100
output_size = 2
batch_size = 1000
layer_sizes = [input_size, hidden_size, output_size]
net_size = input_size + hidden_size + output_size

epsilon = 0.1
beta = 1.0
epochs = 30
n_it_neg = 20
n_it_pos = 4

[os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f.endswith(".png")]

act = lambda x: tf.clip_by_value(tf.cast(x, tf.float32), 0.0, 1.0)

x = tf.placeholder(tf.float32, [None, input_size], name="x")
y = tf.placeholder(tf.int32, [None], name="y")

y_data_one_hot = tf.one_hot(y, output_size)


layers = [
    x,
    tf.placeholder(tf.float32, [None, hidden_size], name="hidden"),
    tf.placeholder(tf.float32, [None, output_size], name="output"),
]

biases  = [
    tf.expand_dims(tf.get_variable("b{}".format(id), [size], initializer = bias_init, dtype=tf.float32), 1)
    for id, size in enumerate(layer_sizes)
]

weights = [
    tf.get_variable("W{}".format(id), [size_pre, size_post], initializer = init, dtype=tf.float32) 
    for id, (size_pre, size_post) in enumerate(zip(layer_sizes[:-1],layer_sizes[1:]))
]

def energy(layers):
    squared_norm    =   tf.reduce_sum( [batch_inner(act(layer),act(layer))       for layer      in layers] ) / 2.
    linear_terms    = - tf.reduce_sum( [tf.matmul(act(layer),b)                        for layer,b    in zip(layers, biases)] )
    quadratic_terms = - tf.reduce_sum( [batch_inner(tf.matmul(act(pre), W), act(post)) for pre,W,post in zip(layers[:-1], weights, layers[1:])])
    return squared_norm + linear_terms + quadratic_terms

def cost(layers):
    return tf.reduce_sum((layers[-1] - y_data_one_hot) ** 2, 1)

def total_energy(layers, beta):
    return energy(layers) + beta * cost(layers)

def measure():
    E = tf.reduce_mean(energy(layers))
    C = tf.reduce_mean(cost(layers))
    y_prediction = tf.argmax(layers[-1], 1)
    error        = tf.reduce_mean(tf.cast(tf.not_equal(y_prediction, tf.cast(y, tf.int64)), tf.float32))

    return E, C, error


def free_phase(n_iterations, epsilon):
    def step(step_num, layers):
        E_sum = tf.reduce_sum(energy(layers))
        layers_dot = tf.gradients(-E_sum, layers[1:]) # temporal derivative of the state (free trajectory)
        layers_new = [ layers[0] ] + [ 
            tf.clip_by_value(layer+epsilon*dot,0.,1.) for layer, dot in zip(layers[1:], layers_dot)
        ]
        return step_num+1, layers_new

    step_num = tf.constant(0, dtype=tf.int32)
    
    _, layers_new = tf.while_loop(
        body=lambda *args: step(*args),
        cond=lambda step_num, *_: tf.less(step_num, n_iterations),
        loop_vars=[step_num, layers]
    )
    return layers_new

def weakly_clamped_phase(layers, n_iterations, epsilon, beta):
    def step(step_num, layers):
        F_sum = tf.reduce_sum(total_energy(layers, beta))
        layers_dot = tf.gradients(-F_sum, layers[1:]) # temporal derivative of the state (free trajectory)
        layers_new = [ layers[0] ] + [ 
            tf.clip_by_value(layer+epsilon*dot,0.,1.) for layer, dot in zip(layers[1:], layers_dot)
        ]
        return step_num+1, layers_new

    step_num = tf.constant(0, dtype=tf.int32)
    
    _, layers_weakly_clamped = tf.while_loop(
        body=lambda *args: step(*args),
        cond=lambda step_num, *_: tf.less(step_num, n_iterations),
        loop_vars=[step_num, layers]
    )

    E_mean_free           = tf.reduce_mean(energy(layers))
    E_mean_weakly_clamped = tf.reduce_mean(energy(layers_weakly_clamped))
    
    cost = (E_mean_weakly_clamped-E_mean_free) / beta
    
    
    return layers_weakly_clamped, cost

layers_free = free_phase(n_it_neg, epsilon)
_, loss = weakly_clamped_phase(layers_free, n_it_pos, epsilon, beta)

opt = tf.train.GradientDescentOptimizer(0.0001)
# opt = tf.train.AdamOptimizer(0.0001)

tvars = tf.trainable_variables()
grads = tf.gradients(loss, tvars)
train_step = opt.apply_gradients(zip(grads, tvars))


sess = tf.Session()
sess.run(tf.global_variables_initializer())


x_values, y_values = get_data()

n_batches_train = (4 * x_values.shape[0]/5) / batch_size
n_batches_valid = (1 * x_values.shape[0]/5) / batch_size

layers_vals = [
    None,
    np.zeros((batch_size, hidden_size)),
    np.zeros((batch_size, output_size)),
]

for e in xrange(20):
    stat = []
    for index in xrange(n_batches_train):
        x_v = x_values[index * batch_size: (index + 1) * batch_size]
        y_v = y_values[index * batch_size: (index + 1) * batch_size]
        
        Mv, lv, _ = sess.run(
            [
                measure(),
                layers_free,
                train_step
            ] , {
                x: x_v, 
                y: y_v, 
                layers[1]: layers_vals[1],
                layers[2]: layers_vals[2],
            }
        )

        layers_vals[1] = lv[1]
        layers_vals[2] = lv[2]

        stat.append(Mv)

    stat = np.asarray(stat)
    mstat = np.mean(stat, 0)
    
    print "Epoch {}, E {}, C {}, error {}".format(
        e, 
        mstat[0],
        mstat[1],
        mstat[2],
    )



