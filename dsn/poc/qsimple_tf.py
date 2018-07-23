
from poc.datasets import *
from poc.util import *

import tensorflow as tf

np.random.seed(12)
tf.set_random_seed(12)

ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape

x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")

layer_size = 100

W0v = np.random.random((input_size, layer_size)).astype(np.float32)
W1v = np.random.random((layer_size, output_size)).astype(np.float32)

# W0v = np.ones((input_size, layer_size))
W0 = tf.Variable(W0v)
W1 = tf.Variable(W1v)


c0 = tf.nn.l2_loss(tf.matmul(tf.transpose(W0), W0) - tf.eye(layer_size))
c1 = tf.nn.l2_loss(tf.matmul(tf.transpose(W1), W1) - tf.eye(output_size))


a0 = tf.nn.relu(tf.matmul(x, W0))
a1 = tf.nn.sigmoid(tf.matmul(a0, W1))

rec_loss = tf.nn.l2_loss()
loss = tf.reduce_sum(a)



dW0_inner = 2.0 * tf.tile(tf.reduce_sum(W0, axis=1, keepdims=True), (1, layer_size))
dW0_c = 2.0 * tf.matmul(dW0_inner, v) / layer_size



# a = tf.matmul(tf.transpose(W0), W0)
# v = a

dW0 = tf.gradients(a, [W0])[0]



opt = tf.train.GradientDescentOptimizer(learning_rate=0.001)

apply_grad_step = opt.apply_gradients(((dW0, W0), ))

sess = tf.Session()
sess.run(tf.global_variables_initializer())


for epoch in range(1):
    W0v, dW0v, dW0_c_v, loss_v, _= sess.run((W0, dW0, dW0_c, loss, apply_grad_step, ))

    print(epoch, loss_v)


# shm(2.0*np.tile(np.sum(W0v, 1), (layer_size, 1)).T, dW0v)