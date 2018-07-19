

from poc.datasets import *
from poc.util import *

import tensorflow as tf

def sigmoid_deriv(x):
    v = tf.sigmoid(x)
    return v * (1.0 - v)

def ltd(a_ff, a_fb):
    ltd = tf.zeros(tf.shape(a_ff))
    ltd = tf.where(a_fb < 1e-10, a_ff, ltd)
    return ltd

def relu_deriv(x):
    a = tf.nn.relu(x)
    da = tf.where(a > 0.0, tf.ones(tf.shape(a)), tf.zeros(tf.shape(a)))
    return da

def sigmoid_inv(x):
    return tf.log(x/(1.0 - x + 1e-09) + 1e-09)


np.random.seed(12)
tf.set_random_seed(12)

ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape


x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")


weight_factor = 0.01
layer_size = 500

W0v = np.random.random((input_size, layer_size)).astype(np.float32) - 0.5
W1v = np.random.random((layer_size, output_size)).astype(np.float32) - 0.5

# W0v = random_orth((input_size, layer_size)).astype(np.float32)
# W1v = random_orth((layer_size, output_size)).astype(np.float32)

b0v = np.random.random((layer_size,)).astype(np.float32) - 0.5
b1v = np.random.random((output_size,)).astype(np.float32) - 0.5


W0 = tf.Variable(W0v)
W1 = tf.Variable(W1v)
b0 = tf.Variable(b0v)
b1 = tf.Variable(b1v)

##

orth_cost = (
    tf.nn.l2_loss(tf.matmul(tf.transpose(W0), W0) - tf.eye(layer_size)) +
    tf.nn.l2_loss(tf.matmul(tf.transpose(W1), W1) - tf.eye(output_size))
)

dW0_orth, dW1_orth = tf.gradients(orth_cost, [W0, W1])


u0 = tf.matmul(x, W0) + b0
a0 = tf.nn.sigmoid(u0)

u1 = tf.matmul(a0, W1) + b1
a1 = tf.nn.sigmoid(u1)

a1_fb = y
u0_fb = tf.matmul(a1_fb, tf.transpose(W1))
a0_fb = tf.nn.sigmoid(u0_fb)


du0_mp = a0_fb - a0
du1_mp = y - a1

loss = tf.reduce_sum(tf.square(a1 - y) / 2.0)

du0, du1 = tf.gradients(loss, [u0, u1])
dW0_orth, dW1_orth = tf.gradients(orth_cost, [W0, W1])

gr = [du0, du1]

# baseline
# 0.0243 test, 100 epoch, lrate 0.0001, adam, weight_factor = 1.0

# simple mp
# 0.1235 test, 100 epoch, --//--

du0, du1 = -du0_mp, -du1_mp


dW0 = tf.matmul(tf.transpose(x), du0) #+ 0.1*dW0_orth
db0 = tf.reduce_sum(du0, 0)
dW1 = tf.matmul(tf.transpose(a0), du1) #+ 0.1*dW1_orth
db1 = tf.reduce_sum(du1, 0)


opt = tf.train.GradientDescentOptimizer(learning_rate=0.0001)
# opt = tf.train.AdamOptimizer(learning_rate=0.001)
# opt = tf.train.RMSPropOptimizer(learning_rate=0.0001)

apply_grad_step = tf.group(
    opt.apply_gradients([
        (dW0, W0),
        (dW1, W1),
        (db0, b0),
        (db1, b1),
    ])
)

class_error_rate = tf.reduce_mean(
    tf.cast(tf.not_equal(tf.argmax(a1, 1), tf.argmax(y, 1)), tf.float32)
)


xv, yv = ds.next_train_batch()

sess = tf.Session()
sess.run(tf.global_variables_initializer())

epochs = 200
train_metrics, test_metrics = (
    np.zeros((epochs, 3)),
    np.zeros((epochs, 3))
)

for epoch in range(epochs):
    for _ in range(ds.train_batches_num):
        xv, yv = ds.next_train_batch()

        u, a, loss_v, class_error_rate_v, orth_cost_v, gr0, _ = sess.run([
            [u0, u1],
            [a0, a1],
            loss,
            class_error_rate,
            orth_cost,
            gr,
            apply_grad_step,
        ], {x: xv, y: yv})

        train_metrics[epoch] += (
            loss_v / ds.train_batches_num,
            class_error_rate_v / ds.train_batches_num,
            orth_cost_v / ds.train_batches_num
        )

    for _ in range(ds.test_batches_num):
        xtv, ytv = ds.next_test_batch()

        ut, at, loss_t_v, class_error_rate_t_v, orth_cost_t_v = sess.run([
            [u0, u1],
            [a0, a1],
            loss,
            class_error_rate,
            orth_cost,
        ], {x: xtv, y: ytv})

        test_metrics[epoch] += (
            loss_t_v / ds.test_batches_num,
            class_error_rate_t_v / ds.test_batches_num,
            orth_cost_t_v / ds.test_batches_num,
        )


    if epoch % 5 == 0:
        print("{}, train {:.4f} {:.4f} {:.4f}, test {:.4f} {:.4f} {:.4f}".format(
            epoch,
            train_metrics[epoch][0],
            train_metrics[epoch][1],
            train_metrics[epoch][2],
            test_metrics[epoch][0],
            test_metrics[epoch][1],
            test_metrics[epoch][2]
        ))
