import tensorflow as tf
import numpy as np
from sklearn.datasets import make_classification
from util import *

def get_toy_data(size=2000, n_classes=2, seed=2):
    x_values, y_values = make_classification(
        n_samples=size,
        n_features=2,
        n_informative=2,
        n_redundant=0,
        n_repeated=0,
        n_clusters_per_class=2,
        n_classes=n_classes,
        random_state=seed
    )
    return x_values, y_values

input_size = 2
weight_init_factor = 1.0
# act = tf.nn.relu
act = tf.nn.sigmoid
lrate = 2e-03
seed = 1
tf.set_random_seed(seed)
np.random.seed(seed)
batch_size = 2000
epochs = 2000

input = tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input")


net_size = [100, 2, 100]

vars = []
for ni, layer_size in enumerate(net_size):
	prev_layer_size = input_size if ni == 0 else net_size[ni-1]
	
	vars.append((
		tf.get_variable("W{}".format(ni), (prev_layer_size, layer_size), 
	    	initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor)
		),
		tf.get_variable("b{}".format(ni), (layer_size,), 
	    	initializer=tf.zeros_initializer()
		),
	))

Wo = tf.get_variable("Wo", (net_size[-1], input_size), 
	initializer=tf.uniform_unit_scaling_initializer(factor=weight_init_factor)
)
bo = tf.get_variable("bo", (input_size,), 
	initializer=tf.zeros_initializer()
)

net_outputs = []
for ni, (W, b) in enumerate(vars):
	prev_output = input if ni == 0 else net_outputs[ni-1]
	
	net_outputs.append(
		act(tf.matmul(prev_output, W) + b)
	)

output = tf.matmul(net_outputs[-1], Wo) + bo

loss = tf.nn.l2_loss(input - output)


optimizer = tf.train.AdamOptimizer(lrate)
opt_step = optimizer.minimize(loss)

sess = tf.Session()
sess.run(tf.global_variables_initializer())

x_v, y_v = get_toy_data(batch_size, 2, seed)

for e in xrange(epochs):

	o = sess.run(
		net_outputs + [output, loss, opt_step], 
		{
			input: x_v
		}
	)
	loss_v = o[-2]
	net_outputs_v = o[:-2]

	print "Epoch {}, MSE {:}".format(
        e, 
        loss_v
    )

pl = lambda d: shs(d[np.where(y_v==0)], d[np.where(y_v==1)], labels=["red", "green"])
