
import tensorflow as tf
from function import function
import numpy as np
from util import *
import time

import tensorflow.contrib.layers as tcl


x_dim, z_dim = 2, 2
scale = 1.0
d_iters = 5

def norm(shape, mean, sd):
    return np.asarray(mean) + np.asarray(sd) * np.random.randn(*shape)

def g_net(z):
    x0 = function(z, size=(100, 50,), name="g_net", act=tf.nn.relu)
    return function(x0, name="g_net/out", size=x_dim)

def d_net(x, reuse=True):
    act = lambda x: tf.nn.relu(tcl.batch_norm(x))
    
    d0 = function(x, size=(100, 50,), name="d_net", act=act, reuse=reuse)
    return function(d0, size=1, name="d_net/out", reuse=reuse, use_bias=False)



x = tf.placeholder(tf.float32, [None, x_dim], name='x')
z = tf.placeholder(tf.float32, [None, z_dim], name='z')


x_ = g_net(z)

d = d_net(x, reuse=False)
d_ = d_net(x_)


g_loss = - tf.reduce_mean(d_)
d_loss = tf.reduce_mean(d_) - tf.reduce_mean(d)

epsilon = tf.random_uniform([], 0.0, 1.0)

x_interm = epsilon * x + (1.0 - epsilon) * x_
d_interm = d_net(x_interm)

ddx_s = tf.gradients(d_interm, x_interm)[0]
ddx = tf.sqrt(tf.reduce_sum(tf.square(ddx_s), axis=1) + 1e-07)
ddx = tf.reduce_mean(tf.square(ddx - 1.0) * scale)

d_loss = d_loss + ddx

g_vars = [v for v in tf.global_variables() if v.name.startswith("g_net")]
d_vars = [v for v in tf.global_variables() if v.name.startswith("d_net")]

d_opt, g_opt = None, None
with tf.control_dependencies(tf.get_collection(tf.GraphKeys.UPDATE_OPS)):
    # d_train_opt = tf.train.AdamOptimizer(learning_rate=1e-03, beta1=0.5, beta2=0.9)
    # d_grad_and_vars = d_train_opt.compute_gradients(d_loss, var_list=d_vars)
    # d_grad_and_vars = [
    #     (tf.clip_by_average_norm(g, 0.1), v) for g, v in d_grad_and_vars
    # ]
    # d_opt = d_train_opt.apply_gradients(d_grad_and_vars)

    d_opt = tf.train.AdamOptimizer(learning_rate=1e-03, beta1=0.5, beta2=0.9)\
        .minimize(d_loss, var_list=d_vars)
    g_opt = tf.train.AdamOptimizer(learning_rate=1e-03, beta1=0.5, beta2=0.9)\
        .minimize(g_loss, var_list=g_vars)

    # d_opt = tf.train.RMSPropOptimizer(learning_rate=1e-03)\
    #     .minimize(d_loss, var_list=d_vars)
    # g_opt = tf.train.RMSPropOptimizer(learning_rate=1e-03)\
    #     .minimize(g_loss, var_list=g_vars)


sess = tf.Session()
sess.run(tf.global_variables_initializer())

batch_size = 800

z_sample = lambda: np.random.uniform(-1.0, 1.0, [batch_size, z_dim])
x_sample = lambda: np.concatenate((
    norm((batch_size/4, x_dim), [0.0, 2.0], [2.0, 0.1]),
    norm((batch_size/4, x_dim), [2.0, 0.0], [0.1, 2.0]),
    norm((batch_size/4, x_dim), [2.0, 2.0], [0.2, 0.2]),
    norm((batch_size/4, x_dim), [0.0, 0.0], [0.2, 0.2])
))

sort_by_mean = lambda bx: bx[[v[0] for v in sorted(enumerate(np.mean(bx, 1)), key=lambda x: x[1])]]

start_time = time.time()

for e in xrange(1000):    
    for _ in range(d_iters):
        bx = x_sample()
        bz = z_sample()
        _, d_fake_v, d_v = sess.run([d_opt, d_, d], feed_dict={x: bx, z: bz})
        
    bz = z_sample()
    _, x_hat_v = sess.run([g_opt, x_], feed_dict={z: bz, x: bx})

    if e % 100 == 0:
        bx = x_sample()
        bz = z_sample()

        d_loss_v = sess.run(
            d_loss, feed_dict={x: bx, z: bz}
        )
        g_loss_v, x_hat_v = sess.run(
            [g_loss, x_], feed_dict={z: bz, x: bx}
        )
        print('Iter [%8d] Time [%5.4f] d_loss [%.4f] g_loss [%.4f], error [%.4f]' %
                (e, time.time() - start_time, d_loss_v, g_loss_v, np.mean(np.square(sort_by_mean(x_hat_v) - sort_by_mean(bx)))))

shs(x_hat_v, x_sample(), labels=["red", "green"])
