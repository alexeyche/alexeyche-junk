
import tensorflow as tf
from function import function
import numpy as np
from util import *
import time

x_dim, z_dim = 2, 2
scale = 10.0
d_iters = 3

def norm(shape, mean, sd):
    return np.asarray(mean) + np.asarray(sd) * np.random.randn(*shape)


x = tf.placeholder(tf.float32, [None, x_dim], name='x')
z = tf.placeholder(tf.float32, [None, z_dim], name='z')


x_hat = function(
    function(z, size=(100, 10), name="g_net", layers_num=2, act=tf.nn.tanh), 
    name="g_net/out",
    size=x_dim,
)

discr_net = lambda x_input, reuse: function(
    function(x_input, size=(100, 10), name="d_net", layers_num=2, act=tf.nn.tanh, reuse=reuse), 
    size=1,
    name="d_net/out",
    reuse=reuse,
)

d_hat = discr_net(x_hat, reuse=False)
d = discr_net(x, reuse=True)

g_loss = tf.reduce_mean(d_hat)
d_loss = tf.reduce_mean(d) - tf.reduce_mean(d_hat)

epsilon = tf.random_uniform([], 0.0, 1.0)

x_interm = epsilon * x + (1.0 - epsilon) * x_hat
d_interm = discr_net(x_interm, reuse=True)

ddx_s = tf.gradients(d_interm, x_interm)[0]
ddx = tf.sqrt(tf.reduce_sum(tf.square(ddx_s), axis=1) +  1e-05)
ddx = tf.reduce_mean(tf.square(ddx - 1.0) * scale)

d_loss = d_loss + ddx

g_vars = [v for v in tf.global_variables() if v.name.startswith("g_net")]
d_vars = [v for v in tf.global_variables() if v.name.startswith("d_net")]

d_adam, g_adam = None, None
with tf.control_dependencies(tf.get_collection(tf.GraphKeys.UPDATE_OPS)):
    d_adam = tf.train.AdamOptimizer(learning_rate=1e-3, beta1=0.5, beta2=0.9)\
        .minimize(d_loss, var_list=d_vars)
    g_adam = tf.train.AdamOptimizer(learning_rate=1e-3, beta1=0.5, beta2=0.9)\
        .minimize(g_loss, var_list=g_vars)


sess = tf.Session()
sess.run(tf.global_variables_initializer())

batch_size = 400

z_sample = lambda: np.random.uniform(-1.0, 1.0, [batch_size, z_dim])
x_sample = lambda: np.concatenate((
    norm((batch_size/4, x_dim), [0.0, 2.0], [2.0, 0.1]),
    norm((batch_size/4, x_dim), [2.0, 0.0], [0.1, 2.0]),
    norm((batch_size/4, x_dim), [2.0, 2.0], [0.2, 0.2]),
    norm((batch_size/4, x_dim), [0.0, 0.0], [0.2, 0.2])
))


start_time = time.time()

for e in xrange(1000):    
    for _ in range(0, d_iters):
        bx = x_sample()
        bz = z_sample()
        sess.run(d_adam, feed_dict={x: bx, z: bz})
        
    bz = z_sample()
    _, x_hat_v = sess.run([g_adam, x_hat], feed_dict={z: bz, x: bx})

    if e % 100 == 0:
        bx = x_sample()
        bz = z_sample()

        d_loss_v = sess.run(
            d_loss, feed_dict={x: bx, z: bz}
        )
        g_loss_v = sess.run(
            g_loss, feed_dict={z: bz}
        )
        print('Iter [%8d] Time [%5.4f] d_loss [%.4f] g_loss [%.4f]' %
                (e, time.time() - start_time, d_loss_v, g_loss_v))
