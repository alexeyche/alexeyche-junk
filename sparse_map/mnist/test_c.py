import tensorflow as tf
import numpy as np
from sklearn.datasets import make_classification
from util import *

from mnist.layer import AutoEncoder
from tensorflow.examples.tutorials.mnist import input_data
import matplotlib.pyplot as plt
import sklearn.decomposition as dec

def get_cmap(n, name='hsv'):
    '''Returns a function that maps each index in 0, 1, ..., n-1 to a distinct 
    RGB color; the keyword argument name must be a standard mpl colormap name.'''
    return plt.cm.get_cmap(name, n)

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

input_size = 28*28
weight_init_factor = 1.0
# act = tf.nn.relu
# act = tf.nn.sigmoid
# act = tf.nn.tanh
act = tf.identity

lrate = 1e-04
seed = 2
tf.set_random_seed(seed)
np.random.seed(seed)
batch_size = 1000
epochs = 1000


# l = AutoEncoder("ae", [50, 2, 50], act=act, tied_weights=False)
# l = AutoEncoder("ae", [100, 50, 2, 50, 100], act=act, tied_weights=False)
# l = AutoEncoder("ae", [100, 3, 100], act=act, tied_weights=False)

# l = [
#     AutoEncoder("ae", [100, 3, 100], act=act, tied_weights=False)
# ]

l = [
    AutoEncoder("ae0", [100], act=act, tied_weights=True),
    AutoEncoder("ae1", [3], act=act, tied_weights=True),
    AutoEncoder("ae2", [100], act=act, tied_weights=True),
]

input = tf.placeholder(tf.float32, shape=(batch_size, input_size), name="Input")

nos = []
losses = []
dv_vs = []
for ll in l:
    no, loss, dv_v = ll(nos[-1][0] if len(nos) > 0 else input)

    nos.append(no)
    losses.append(loss)
    dv_vs.append(dv_v)


optimizer = tf.train.AdamOptimizer(lrate)

opt_steps = [optimizer.apply_gradients(dv_v) for dv_v in dv_vs]

#############

sess = tf.Session()
sess.run(tf.global_variables_initializer())


mnist = input_data.read_data_sets("/Users/aleksei/tmp/MNIST_data/", one_hot=False)


lacc = []
for e in xrange(epochs):
    x_v, y_v = mnist.train.next_batch(batch_size)
    o = sess.run(nos + losses + opt_steps, {input: x_v})

    no_v = o[:len(nos)]
    losses_v = o[len(nos):(len(nos)+len(losses))]

    lacc.append(losses_v)
    if e % 100 == 0:
        print "Epoch {}, MSE {:}".format(
            e, 
            losses_v
        )

lacc = np.asarray(lacc)

v = pca(no_v[1][0])
cf = get_cmap(10)
plt.scatter(v[:,0], v[:,1], c=cf(y_v))

ii=300; shm(no_v[-1][-1][ii].reshape((28, 28)), x_v[ii].reshape((28, 28)), show=False)

plt.show()


# pl = lambda d: shs(d[np.where(y_v==0)], d[np.where(y_v==1)], labels=["red", "green"])

