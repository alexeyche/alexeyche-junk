
import collections

import tensorflow as tf
from tensorflow.python.ops import rnn_cell as rc
from util import fun

_VAEOutputTuple = collections.namedtuple("VAEOutputTuple", ("prior_mu", "prior_sigma", "mu", "sigma", "post_mu", "post_sigma", "post_alpha"))
_RNNInputTuple = collections.namedtuple("RNNInputTuple", ("x", "z"))


class VAEOutputTuple(_VAEOutputTuple):
    __slots__ = ()

class RNNInputTuple(_RNNInputTuple):
    __slots__ = ()

def encode(x, h, generator, **kwargs):
    x_transformed = kwargs["x_transformed"]
    z_dim = kwargs["z_dim"]
    phi_interm = kwargs["phi_interm"]
    prior_interm = kwargs["prior_interm"]
    weight_factor = kwargs.get("weight_factor", 1.0)
    layers_num = kwargs.get("layers_num", 1)
    batch_size = x.get_shape().as_list()[0]
    
    x_t = fun(x, nout = x_transformed, act = tf.nn.relu, name = "x_transformed", weight_factor = weight_factor, layers_num = layers_num)

    prior = fun(h, nout = prior_interm, act = tf.nn.relu, name = "prior", weight_factor = weight_factor, layers_num = layers_num)
    prior_mu = fun(prior, nout = z_dim, act = tf.identity, name = "prior_mu", weight_factor = weight_factor)
    prior_sigma = fun(prior, nout = z_dim, act = tf.nn.softplus, name = "prior_sigma", weight_factor = weight_factor)

    phi = fun(x_t, h, nout = phi_interm, act = tf.nn.relu, name = "phi", weight_factor = weight_factor, layers_num = layers_num)
    z_mu = fun(phi, nout = z_dim, act = tf.identity, name = "z_mu", weight_factor = weight_factor)
    z_sigma = fun(phi, nout = z_dim, act = tf.nn.softplus, name = "z_sigma", weight_factor = weight_factor)

    epsilon = tf.random_normal((batch_size, z_dim), name='epsilon')

    z = tf.cond(
        generator, 
        lambda: prior_mu + tf.exp(prior_sigma) * epsilon, 
        lambda: z_mu + tf.exp(z_sigma) * epsilon
    )

    return z, z_mu, z_sigma, prior_mu, prior_sigma, x_t

def decode(z, **kwargs):
    z_interm = kwargs["z_interm"]
    n_mix = kwargs["n_mix"]
    out_interm = kwargs["out_interm"]
    weight_factor = kwargs.get("weight_factor", 1.0)
    layers_num = kwargs.get("layers_num", 1)

    z_t = fun(z, nout = z_interm, act = tf.nn.relu, name = "z_transformed", weight_factor = weight_factor, layers_num = layers_num)

    output_t = fun(z_t, nout = out_interm, act = tf.nn.relu, name = "out_transform", weight_factor = weight_factor, layers_num = layers_num)
    post_mu = fun(output_t, nout = n_mix, act =tf.identity, name = "out_mu", weight_factor = weight_factor)
    post_sigma = fun(output_t, nout = n_mix, act =tf.nn.softplus, name = "out_sigma", weight_factor = weight_factor)
    post_alpha = fun(output_t, nout = n_mix, act =tf.nn.softmax, name = "out_alpha", weight_factor = weight_factor)
    return post_mu, post_sigma, post_alpha, z_t



class VAECell(rc.RNNCell):
    def __init__(self, base_cell, generator, config):
        self._base_cell = base_cell
        self._num_units = self._base_cell.state_size
        self._generator = generator
        self.config = config

    @property
    def state_size(self):
        return self._base_cell.state_size

    @property
    def output_size(self):
        z_dim = self.config["z_dim"]
        n_mix = self.config["n_mix"]
        return VAEOutputTuple(z_dim, z_dim, z_dim, z_dim, n_mix, n_mix, n_mix)

    def __call__(self, x, h, scope=None):
        batch_size = x.get_shape().as_list()[0]
    
        n_mix = self.config["n_mix"]
        x_transformed = self.config["x_transformed"]
        weight_factor = self.config.get("weight_factor", 1.0)
        layers_num = self.config.get("layers_num", 1)

        z, z_mu, z_sigma, prior_mu, prior_sigma, x_t = encode(x, h, self._generator, **self.config)
        post_mu, post_sigma, post_alpha, z_t = decode(z, **self.config)

        epsilon_gen = tf.random_normal((batch_size, n_mix), name='epsilon_gen')
        x_sampled = tf.reduce_sum(post_alpha*(post_mu + epsilon_gen * post_sigma), 1, keep_dims=True)

        x_t_gen = fun(x_sampled, nout = x_transformed, act = tf.nn.relu, name = "x_transformed",
            weight_factor = weight_factor, layers_num = layers_num, reuse=True
        )

        # x_c = tf.cond(
        #     self._generator,
        #     lambda: tf.concat_v2([x_t_gen, z_t], 1),
        #     lambda: tf.concat_v2([x_t, z_t], 1)
        # )
        x_c = tf.cond(
            self._generator,
            lambda: x_t_gen,
            lambda: x_t,
        )
        # x_c = tf.concat_v2([x_t, z_t], 1)
        _, new_h = self._base_cell(x_c, h)

        return VAEOutputTuple(
            prior_mu,
            prior_sigma,
            z_mu,
            z_sigma,
            post_mu, 
            post_sigma, 
            post_alpha), new_h
