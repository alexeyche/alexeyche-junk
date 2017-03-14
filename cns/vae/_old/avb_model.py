from log import setup_log
setup_log()

import tensorflow as tf
from util import fun, kl_divergence_gauss
import numpy as np
ds = tf.contrib.distributions
import collections
from matplotlib import pyplot as plt

import logging
import sys




_DistrOutput = collections.namedtuple("DistrOutput", ("sample", "params"))

def plot_poiss(v, c):
    v_res = []
    cols = []
    for s_id in xrange(v.shape[1]):
        spikes = np.where(v[:, s_id])[0]
        cols.append(c[spikes])
        v_t = np.asarray([
            spikes, 
            np.tile([s_id], len(spikes))
        ])
        v_res.append(v_t)

    v_res = np.concatenate(v_res, axis=1)
    cols = np.concatenate(cols)
    plt.scatter(v_res[0,:], v_res[1,:], c=cols)



def sample_mog(batch_size, n_mixture=8, std=0.1, radius=1.0):
    thetas = np.linspace(0, 2 * np.pi, n_mixture)
    xs, ys = radius * np.sin(thetas), radius * np.cos(thetas)
    cat = ds.Categorical(tf.zeros(n_mixture))
    comps = [ds.MultivariateNormalDiag([xi, yi], [std, std]) for xi, yi in zip(xs.ravel(), ys.ravel())]
    data = ds.Mixture(cat, comps)
    return data.sample_n(batch_size)



class DistrOutput(_DistrOutput):
    __slots__ = ()


class Distribution(object):
    def __init__(self, shape_tuple, name):
        assert len(shape_tuple) == 2, "Need tuple batch_size x dim as input"
        self._dim = shape_tuple[1]
        self._batch_size = shape_tuple[0]
        self._name = name

    def generate(self, *conditionals, **kwargs):
        raise NotImplementedError()

    def generate_prior(self):
        raise NotImplementedError()





class NormalDistribution(Distribution):
    def __init__(self, shape_tuple, name = "N"):
        super(NormalDistribution, self).__init__(shape_tuple, name)

    def generate(self, *conditionals, **kwargs):
        reuse = kwargs.get("reuse", False)
        conditionals = list(reversed(list(conditionals)))
        config = kwargs["config"]
        
        logging.info(
            "\t{}: Generating mu and sigma, conditioning on: {}".format(
            self._name, 
            ", ".join(["{}".format(c.name) for c in conditionals])
        ))
        
        x_t = fun(*conditionals, nout = config.x_interm, act = tf.nn.elu, name = "encode_to_{}".format(self._name), layers_num=config.layers_num, config=config, reuse=reuse)
        
        mean = fun(x_t, nout = self._dim, act = tf.identity, name = "{}_mean".format(self._name), reuse = reuse, config = config, layers_num=1)
        logvar = fun(x_t, nout = self._dim, act = tf.nn.softplus, name = "{}_logvar".format(self._name), reuse = reuse, config = config, layers_num=1)

        N0 = tf.random_normal((self._batch_size, self._dim))
        stddev = tf.exp(0.5 * logvar)
        
        return (
            DistrOutput(tf.add(mean, stddev * N0, name=self._name), ds.Normal(mean, stddev)),
        )
    
    def generate_prior(self, **kwargs):
        return (
            DistrOutput(tf.random_normal((self._batch_size, self._dim), name="{}_prior".format(self._name)), ds.Normal(0.0, 1.0)),
        )


class BernoulliDistribution(Distribution):
    def __init__(self, shape_tuple, name = "bernoulli"):
        super(BernoulliDistribution, self).__init__(shape_tuple, name)
        self._bias = -1.0

    def generate(self, *conditionals, **kwargs):
        reuse = kwargs.get("reuse", False)
        conditionals = list(reversed(list(conditionals)))
        config = kwargs["config"]

        x_t = fun(*conditionals, nout = config.x_interm, act = tf.nn.elu, name = "encode_to_{}".format(self._name), layers_num=config.layers_num, config=config, reuse=reuse)

        rate_not_norm = fun(x_t, nout = self._dim, act = tf.identity, name = "{}_rate".format(self._name), reuse = reuse, config = config, layers_num=1)
        rate = tf.nn.sigmoid(self._bias + rate_not_norm)
        U0 = tf.random_uniform((self._batch_size, self._dim))
        res = tf.where(rate > U0, tf.ones((self._batch_size, self._dim)), tf.zeros((self._batch_size, self._dim)), name=self._name)

        return (
            DistrOutput(res, ds.Bernoulli(p=rate)),
        )

    def generate_prior(self, **kwargs):
        rate_not_norm = tf.random_normal((self._batch_size, self._dim))
        rate = tf.nn.sigmoid(self._bias + rate_not_norm)

        U0 = tf.random_uniform((self._batch_size, self._dim))
        res = tf.where(rate > U0, tf.ones((self._batch_size, self._dim)), tf.zeros((self._batch_size, self._dim)), name="{}_prior".format(self._name))
        
        return (
            DistrOutput(res, ds.Bernoulli(p=rate)),
        )


class HierarchicalDistribution(Distribution):
    def __init__(self, *distributions):
        self._distributions = distributions


    def generate(self, *conditionals, **kwargs):
        generated = list(conditionals)
        outputs = []
        for distr in self._distributions:
            curr_outputs = distr.generate(*generated, **kwargs)
            
            generated += [o.sample for o in curr_outputs]
            outputs += list(curr_outputs)

        return outputs

    def generate_prior(self, **kwargs):
        assert len(self._distributions) > 0, "Need to define distributions"
        
        high_level_distr_tuple = self._distributions[0].generate_prior(**kwargs)
        
        generated = [o.sample for o in high_level_distr_tuple]
        
        outputs = list(high_level_distr_tuple)

        for distr in self._distributions[1:]:            
            curr_outputs = distr.generate(*generated, **kwargs)

            generated += [o.sample for o in curr_outputs]
            outputs += list(curr_outputs)
        return outputs



class RunMode(object):
    VAE = "vae_mode"
    AVB = "avb_mode"

class AvbModel(object):
    def __init__(self, config, distribution):
        self._config = config
        self._distribution = distribution


    def encode(self, x, reuse=False):
        return self._distribution.generate(x, reuse=reuse, config = self._config)    

    def decode(self, *z):
        z_t = fun(*z, nout = self._config.z_interm, act = tf.nn.elu, name = "decode_z_transformed", layers_num=self._config.layers_num, config=self._config)
        post_mu = fun(z_t, nout = self._config.output_dim, act = tf.identity, name = "decode_out_mu", layers_num = 1, config=self._config)
        return post_mu

    def discriminate(self, x, *latent, **kwargs):
        reuse = kwargs.get("reuse", False)

        a_t = fun(x, *latent, nout = self._config.a_interm, act = tf.nn.elu, name = "discriminator_transformed", layers_num=self._config.layers_num, config=self._config, reuse=reuse)
        a_out = fun(a_t, nout = self._config.output_dim, act = tf.identity, name = "discriminator_out", reuse=reuse, layers_num = 1, config=self._config)
        return a_out

    def get_distribution(self):
        return self._distribution

    def get_config(self):
        return self._config

    @staticmethod
    def build(model, input, mode = RunMode.AVB, full_sample=False):
        logging.info("Building model, {}".format(mode))
        logging.info("Encoding ... ")
        doutputs = model.encode(input)   # q(z | x)
        if full_sample:
            logging.info("Decoding ... conditioned on: {}".format(", ".join(
                [o.sample.name for o in doutputs]
            )))
            post_mu = model.decode(*[o.sample for o in doutputs])
        else:
            logging.info("Decoding ... conditioned on: {}".format(doutputs[-1].sample.name))
            post_mu = model.decode(doutputs[-1].sample)
            

        distribution, config = model.get_distribution(), model.get_config()
        
        logging.info("Generating prior ...")

        dprior_outputs = distribution.generate_prior(config=config, reuse=True)
        
        adv = model.discriminate(input, *[o.sample for o in doutputs])
        adv_prior = model.discriminate(input, *[o.sample for o in dprior_outputs], reuse=True)
        
        neg_log_x_z = tf.nn.l2_loss(post_mu - input)

        if mode == RunMode.VAE:
            out_shape = post_mu.get_shape()
            kl = tf.zeros(out_shape[:-1])
            for dout, dprior_out in zip(doutputs, dprior_outputs):
                kl += tf.reduce_mean(ds.kl(dout.params, dprior_out.params), out_shape.ndims-1)
        elif mode == RunMode.AVB:
            kl = adv    
        else:
            raise Exception("Unknown mode")

        net_loss = tf.reduce_mean(kl + neg_log_x_z)
        adv_loss = tf.reduce_mean(-tf.log(tf.nn.sigmoid(adv)+1e-10) - tf.log(1.0 - tf.nn.sigmoid(adv_prior) + 1e-10))

        vars = tf.trainable_variables()
        net_params = [v for v in vars if not v.name.startswith('discriminator')]
        adv_params = [v for v in vars if v.name.startswith('discriminator')]

        net_step = tf.train.AdamOptimizer(config.lrate).minimize(net_loss, var_list=net_params)
        adv_step = tf.train.AdamOptimizer(config.lrate).minimize(adv_loss, var_list=adv_params)

        return (
            [o.sample for o in doutputs], 
            [o.sample for o in dprior_outputs],
            neg_log_x_z,
            kl, 
            adv_prior,
            post_mu, 
            net_loss, 
            adv_loss, 
            net_step, 
            adv_step
        )
