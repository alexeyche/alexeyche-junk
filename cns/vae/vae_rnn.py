
#!/usr/bin/env python

import os
import numpy as np
import tensorflow as tf
from matplotlib import pyplot as plt
from os.path import join as pj
from glob import glob
from env import Env
import librosa as lr
import collections


from tensorflow.python.ops import variable_scope as vs
from tensorflow.python.ops import rnn
from tensorflow.python.ops import rnn_cell as rc
from tensorflow.python.ops import math_ops as mo
from tensorflow.python.ops import array_ops
from tensorflow.python.framework import dtypes

from util import sm, sl, smooth_matrix, smooth, moving_average, norm, outer, generate_dct_dictionary, fun

def xavier_init(fan_in, fan_out, constant=1):
    low = -constant*np.sqrt(6.0/(fan_in + fan_out))
    high = constant*np.sqrt(6.0/(fan_in + fan_out))
    return tf.random_uniform((fan_in, fan_out),
                             minval=low, maxval=high,
                             dtype=tf.float32)

np.random.seed(10)
tf.set_random_seed(10)

target_sr = 5000

epochs = 1000
lrate = 1e-03
seq_size = 2000

batch_size = 1

weight_factor = 0.5

layers_num = 2

x_transformed = 100

phi_interm = 100
prior_interm = 100

z_dim = 25
z_interm = 100
out_interm = 100

net_size = 100


_VAEOutputTuple = collections.namedtuple("VAEOutputTuple", ("prior_mu", "prior_sigma", "mu", "sigma", "z", "z_transformed", "output"))
_RNNInputTuple = collections.namedtuple("RNNInputTuple", ("x", "z"))


class VAEOutputTuple(_VAEOutputTuple):
    __slots__ = ()

class RNNInputTuple(_RNNInputTuple):
    __slots__ = ()


class VAECell(rc.RNNCell):
    def __init__(self, base_cell):
        self._base_cell = base_cell
        self._num_units = self._base_cell.state_size
        self._generator = False

    @property
    def state_size(self):
        return self._base_cell.state_size

    @property
    def output_size(self):
        return VAEOutputTuple(z_dim, z_dim, z_dim, z_dim, z_dim, z_interm, 1)

    def set_generator(self, gen_flag):
        self._generator = gen_flag

    def __call__(self, x, h, scope=None):
        x_t = fun(x, nout = x_transformed, act = tf.nn.tanh, name = "x_transformed", weight_factor = weight_factor, layers_num = layers_num)
        
        # prior, depends only on state
        
        prior_t = fun(h, nout = prior_interm, act = tf.nn.tanh, name = "prior", weight_factor = weight_factor, layers_num = layers_num)
        prior_mu_t = fun(prior_t, nout = z_dim, act = tf.identity, name = "prior_mu", weight_factor = weight_factor)
        prior_sigma_t = fun(prior_t, nout = z_dim, act = tf.nn.softplus, name = "prior_sigma", weight_factor = weight_factor)
        
        # phi
        phi_t = fun(x_t, h, nout = phi_interm, act = tf.nn.tanh, name = "phi", weight_factor = weight_factor, layers_num = layers_num)        
        phi_mu_t = fun(phi_t, nout = z_dim, act = tf.identity, name = "phi_mu", weight_factor = weight_factor)
        phi_sigma_t = fun(phi_t, nout = z_dim, act = tf.nn.softplus, name = "phi_sigma", weight_factor = weight_factor)        
        
        # z generating
        epsilon = tf.random_normal(tf.shape(z_dim), name='epsilon')

        if not self._generator:
            z = phi_mu_t + phi_sigma_t * epsilon
        else:
            z = prior_mu_t + prior_sigma_t * epsilon

        z_t = fun(z, nout = z_interm, act = tf.nn.tanh, name = "z_transformed", weight_factor = weight_factor, layers_num = layers_num)

        output_t = fun(z_t, h, nout = out_interm, act = tf.nn.tanh, name = "out_transform", weight_factor = weight_factor, layers_num = layers_num)
        output_mu = fun(output_t, nout = 1, act =tf.identity, name = "out_mu", weight_factor = weight_factor)
        # output_sigma = fun(out_t, nout = 1, act = tf.nn.softplus, name = "out_sigma", weight_factor = weight_factor)
            
        if not self._generator:
            x_c = tf.concat_v2([x_t, z_t], 1)
        else:
            x_t_gen = fun(output_mu, nout = x_transformed, act = tf.nn.tanh, name = "x_transformed", weight_factor = weight_factor, layers_num = layers_num)
            x_c = tf.concat_v2([x_t_gen, z_t], 1) 
            
        _, new_h = self._base_cell(x_c, h)
        
        return VAEOutputTuple(
            prior_mu_t, 
            prior_sigma_t, 
            phi_mu_t, 
            phi_sigma_t, 
            z, 
            z_t, 
            output_mu), new_h


env = Env("vae_run")

# cell = VAECell(tf.nn.rnn_cell.BasicRNNCell(net_size))
cell = VAECell(tf.nn.rnn_cell.GRUCell(net_size))

input = tf.placeholder(tf.float32, shape=(seq_size, batch_size, 1), name="Input")
state = tf.placeholder(tf.float32, [batch_size, net_size], name="state")

with tf.variable_scope("rnn") as scope:
    out, finstate = rnn.dynamic_rnn(cell, input, initial_state=state, time_major=True, scope = scope) 


with tf.variable_scope("rnn", reuse = True) as scope:
    cell.set_generator(True)
    out_gen, finstate_gen = rnn.dynamic_rnn(cell, input, initial_state=state, time_major=True, scope = scope) 

kl = tf.reduce_sum(
    0.5 * (
        2.0 * tf.log(out.prior_sigma) - 2.0 * tf.log(out.sigma) +
       (out.sigma**2 + (out.mu - out.prior_mu)**2) / out.prior_sigma**2 - 1.0
    ),
    [1, 2]
)

# error = 0.5 * tf.reduce_mean(tf.square(input_p - out_mu) / out_sigma**2 + 2 * tf.log(out_sigma) + tf.log(2 * np.pi), [1])
 
error = tf.reduce_sum(tf.square(out.output - input), 1)

loss = tf.reduce_mean(error + kl)

optimizer = tf.train.AdamOptimizer(lrate)
# optimizer = tf.train.RMSPropOptimizer(lrate)
# optimizer = tf.train.GradientDescentOptimizer(lrate)


tvars = tf.trainable_variables()
grads_raw = tf.gradients(loss, tvars)
grads, _ = tf.clip_by_global_norm(grads_raw, 5.0)

apply_grads = optimizer.apply_gradients(zip(grads, tvars))

sess = tf.Session()

model_fname = env.run("model.ckpt")
saver = tf.train.Saver()
if len(glob("{}*".format(model_fname))) > 0:
    print "Restoring from {}".format(model_fname)
    saver.restore(sess, model_fname)
    epochs = 0 
else:
    sess.run(tf.global_variables_initializer())


tmp_dir = env.run()
tmp_grad_dir = env.run("grads")
if not os.path.exists(tmp_grad_dir):
    os.makedirs(tmp_grad_dir)

[ os.remove(pj(tmp_dir, f)) for f in os.listdir(tmp_dir) if f[-4:] == ".png" ]
[ os.remove(pj(tmp_grad_dir, f)) for f in os.listdir(tmp_grad_dir) if f[-4:] == ".png" ]

# input_v =  moving_average(np.random.randn(seq_size), 50).reshape(1, seq_size, batch_size, 1)


data_source = []
for f in sorted(os.listdir(env.dataset())):
    if f.endswith(".wav"):
        data_source.append(env.dataset(f))


def read_song(source_id, target_sr):
    song_data_raw, source_sr = lr.load(data_source[source_id])
    song_data = lr.resample(song_data_raw, source_sr, target_sr, scale=True)
    song_data = song_data[1500:(1500+50*seq_size)] #song_data.shape[0]/10]
    song_data, data_denom = norm(song_data, return_denom=True)
    
    return song_data, source_sr, data_denom


def generate(iterations=1):
    state_v = np.zeros((batch_size, net_size))
    
    output = []
    for i in xrange(iterations):
        out_gen_v, finstate_v = sess.run([
            out_gen,
            finstate_gen
        ], {
            input: np.zeros((seq_size, batch_size, 1)),
            state: state_v    
        })
        state_v = finstate_v

        output.append(out_gen_v.output.reshape(seq_size))
    
    return np.concatenate(output)
        
data, source_sr, data_denom = read_song(0, target_sr)


for e in xrange(epochs):
    state_v = np.zeros((batch_size, net_size))
    
    output_data = []
    input_data = []
    perf = []
    for id_start in xrange(0, data.shape[0], seq_size):
        input_v = np.zeros(seq_size)
        
        lb = id_start
        ub = min(id_start+seq_size, data.shape[0])
        
        input_v[:(ub-lb)] = data[lb:ub]

        sess_out = sess.run([
            finstate,
            out, 
            kl, 
            error, 
            loss, 
            apply_grads
            # grads, 
            # grads_raw, 
            # out_sigma
        ], {
            input: input_v.reshape(seq_size, batch_size, 1),
            state: state_v
        })
    
        state_v, out_v, kl_v, error_v, loss_v, _ = sess_out[0:6]
        
        output_data.append(out_v.output.reshape(seq_size)[:(ub-lb)])    
        input_data.append(input_v.reshape(seq_size)[:(ub-lb)])
        perf.append((loss_v, np.mean(kl_v), np.mean(error_v)))


    sl(np.concatenate(input_data), np.concatenate(output_data), file=pj(tmp_dir, "rec_{}.png".format(e)))

    loss_v = np.mean(map(lambda x: x[0], perf))
    kl_v = np.mean(map(lambda x: x[1], perf))
    error_v = np.mean(map(lambda x: x[2], perf))

    # for g, gr, tv in zip(grads_v, grads_raw_v, tvars):
    #     vname = tv.name.replace("/", "_").replace(":","_")

    #     f = pj(tmp_grad_dir, "{}_{}.png".format(vname, e))
    #     fr = pj(tmp_grad_dir, "{}_raw_{}.png".format(vname, e))

    #     work_dims = [s for s in g.shape if s > 1]
    #     if len(work_dims) == 1:
    #         sl(g, file=f)
    #         # sl(gr, file=fr)
    #     elif len(work_dims) == 2:
    #         sm(g, file=f)
    #         # sm(gr, file=fr)


    print "Epoch {}, loss {}, KLD {}, rec.error: {}".format(e, loss_v, kl_v, error_v)

    if e % 10 == 0:
        sl(generate(), file=pj(tmp_dir, "generated_{}.png".format(e)))

    if e % 100 == 0:
        d = generate(10)
        
        sl(d, file=pj(tmp_dir, "generated_long_{}.png".format(e)))

        lr.output.write_wav(
            pj(tmp_dir, "generated_{}.wav".format(e)), 
            lr.resample(d, target_sr, source_sr, scale=True), 
            source_sr
        )
        
saver.save(sess, model_fname)
