import tensorflow as tf
import numpy as np
from sklearn.datasets import make_classification
from util import *
import sklearn.decomposition as dec

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

def pca(a):
	pca = dec.PCA(2)
	return pca.fit(a).transform(a)

input_size = 2
weight_init_factor = 1.0
# act = tf.nn.relu
act = tf.nn.sigmoid
lrate = 1e-02
seed = 1
tf.set_random_seed(seed)
np.random.seed(seed)
batch_size = 2000
epochs = 10000
global_loss = False

input = tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input")


net_size = [100, 100, 100]

# grad_factors = [1.0, 0.1, 0.001, 0.0001]
grad_factors = [1.0, 1.0, 1.0, 1.0]
# grad_factors = [0.0, 0.0, 0.0, 1.0]

vars, loss_and_grads = [], []
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

	if not global_loss:
		l = tf.nn.l2_loss(tf.matmul(net_outputs[-1], tf.transpose(W)) - prev_output)
		dW, db = tf.gradients(l, [W, b])
		
		dW *= grad_factors[ni]
		db *= grad_factors[ni]

		loss_and_grads.append(
			(l, [(dW, W), (db, b)])
		)


output = tf.matmul(net_outputs[-1], Wo) + bo

if global_loss:
	l = tf.nn.l2_loss(input - output)
	flat_vars = [v for t in vars for v in t]
	grads = tf.gradients(l, flat_vars)
	loss_and_grads.append((l, list(zip(grads, flat_vars))))
else:
	l = tf.nn.l2_loss(input - output)
	dWo, dbo = tf.gradients(l, [Wo, bo])
	
	dWo *= grad_factors[-1]
	dbo *= grad_factors[-1]

	loss_and_grads.append((l, [(dWo, Wo), (dbo, bo)]))


# optimizer = tf.train.GradientDescentOptimizer(lrate)
optimizer = tf.train.AdamOptimizer(lrate)

opt_steps = [optimizer.apply_gradients(g_and_v) for _, g_and_v in loss_and_grads]
loss = [l for l, _ in loss_and_grads]

sess = tf.Session()
sess.run(tf.global_variables_initializer())

x_v, y_v = get_toy_data(batch_size, 2, seed)

losses = []
for e in xrange(epochs):

	o = sess.run(
		net_outputs + [output] + loss + opt_steps, 
		{
			input: x_v
		}
	)
	
	loss_v = o[-2*len(opt_steps):-len(opt_steps)]

	losses.append(loss_v)

	# loss_v = o[-2]
	net_outputs_v = o[:-2*len(opt_steps)]
	if e % 100 == 0:
		print "Epoch {}, MSE {:}".format(
	        e, 
	        loss_v
	    )

losses = np.asarray(losses)

pl = lambda d: shs(d[np.where(y_v==0)], d[np.where(y_v==1)], labels=["red", "green"])
