
import tensorflow as tf
from poc.datasets import *
from poc.util import *
from poc.common import *

ds = XorDataset()

(_, input_size), (_, output_size) = ds.train_shape

lil_epsilon = 1.0
big_epsilon = 10.0
weight_factor = 1.0
layer_size = 100 #(100, 100)
threshold = 0.1
net_structure = (100, output_size)

tf.set_random_seed(1)


x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")


net_act = lambda u: tf.cast(tf.greater(u, threshold), tf.float32)

n = Network(input_size, net_structure, weight_factor, net_act, output_act=tf.nn.sigmoid)


sess = tf.Session()
sess.run(tf.global_variables_initializer())


xv, yv = ds.next_train_batch()
act_v = sess.run(act, {x: xv})
