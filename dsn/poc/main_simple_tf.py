

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


np.random.seed(12)
tf.set_random_seed(12)

ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape


x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")


weight_factor = 0.01
layer_size = 500

W0v = weight_factor * np.random.random((input_size, layer_size)).astype(np.float32)
W1v = weight_factor * np.random.random((layer_size, output_size)).astype(np.float32)

b0v = weight_factor * np.random.random((layer_size,)).astype(np.float32)
b1v = weight_factor * np.random.random((output_size,)).astype(np.float32)


W0 = tf.Variable(W0v)
W1 = tf.Variable(W1v)
b0 = tf.Variable(b0v)
b1 = tf.Variable(b1v)

##

u0 = tf.matmul(x, W0) + b0
a0 = tf.nn.softplus(u0)

u1 = tf.matmul(a0, W1) + b1
a1 = tf.nn.softmax(u1)


# loss = tf.reduce_sum(tf.square(a1 - y) / 2.0)
loss = tf.reduce_sum(tf.nn.softmax_cross_entropy_with_logits_v2(logits=u1, labels=y))

gr = tf.gradients(loss, [u0, u1])
gr = [-g for g in gr]

dW0, dW1, db0, db1 = tf.gradients(loss, [W0, W1, b0, b1])

# opt = tf.train.GradientDescentOptimizer(learning_rate=0.0001)
opt = tf.train.AdamOptimizer(learning_rate=0.0001)
# opt = tf.train.RMSPropOptimizer(learning_rate=0.0001)

apply_grad_step = tf.group(
    opt.apply_gradients([
        (dW0, W0),
        (dW1, W1),
        (db0, b0),
        (db1, b1),
    ]),
    tf.group(
        W0.assign(tf.maximum(0.0, W0)),
        W1.assign(tf.maximum(0.0, W1)),
    ),
)

class_error_rate = tf.reduce_mean(
    tf.cast(tf.not_equal(tf.argmax(a1, 1), tf.argmax(y, 1)), tf.float32)
)


xv, yv = ds.next_train_batch()

sess = tf.Session()
sess.run(tf.global_variables_initializer())

epochs = 200
train_metrics, test_metrics = (
    np.zeros((epochs, 2)),
    np.zeros((epochs, 2))
)

for epoch in range(epochs):
    for _ in range(ds.train_batches_num):
        xv, yv = ds.next_train_batch()

        u, a, loss_v, class_error_rate_v, gr0, _ = sess.run([
            [u0, u1],
            [a0, a1],
            loss,
            class_error_rate,
            gr,
            apply_grad_step,
        ], {x: xv, y: yv})

        train_metrics[epoch] += (
            loss_v / ds.train_batches_num,
            class_error_rate_v / ds.train_batches_num
        )

    for _ in range(ds.test_batches_num):
        xtv, ytv = ds.next_test_batch()

        ut, at, loss_t_v, class_error_rate_t_v = sess.run([
            [u0, u1],
            [a0, a1],
            loss,
            class_error_rate
        ], {x: xtv, y: ytv})

        test_metrics[epoch] += (
            loss_t_v / ds.test_batches_num,
            class_error_rate_t_v / ds.test_batches_num
        )


    if epoch % 5 == 0:
        print("{}, train {:.4f} {:.4f}, test {:.4f} {:.4f}".format(
            epoch,
            train_metrics[epoch][0],
            train_metrics[epoch][1],
            test_metrics[epoch][0],
            test_metrics[epoch][1]
        ))
