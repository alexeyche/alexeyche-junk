

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

def threshold(x):
    a = tf.nn.relu(x)
    # return tf.where(a > 0.0, tf.ones(tf.shape(a)), tf.zeros(tf.shape(a)))
    return a

np.random.seed(14)
tf.set_random_seed(14)

ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape


x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")


weight_factor = 0.5
layer_size = 500

# W0v = random_sparse((input_size, layer_size), p=0.75)
# W1v = random_sparse((layer_size, layer_size), p=0.75)
# W2v = random_sparse((layer_size, output_size), p=0.75)


W0v, b0v = xavier_init(input_size, layer_size, const=weight_factor)
W1v, b1v = xavier_init(layer_size, layer_size, const=weight_factor)
W2v, b2v = xavier_init(layer_size, output_size, const=weight_factor)


# W0v = random_orth((input_size, layer_size)).astype(np.float32)
# W1v = random_orth((layer_size, layer_size)).astype(np.float32)
# W2v = random_orth((layer_size, output_size)).astype(np.float32)


# b0v = np.zeros((layer_size,)).astype(np.float32)
# b1v = np.zeros((layer_size,)).astype(np.float32)

# b0v = np.random.random((layer_size,)).astype(np.float32) - 0.5
# b1v = np.random.random((layer_size,)).astype(np.float32) - 0.5


W0 = tf.Variable(W0v)
W1 = tf.Variable(W1v)
W2 = tf.Variable(W2v)
b0 = tf.Variable(b0v)
b1 = tf.Variable(b1v)

##



u0 = tf.matmul(x, W0) + b0
a0 = threshold(u0)

u1 = tf.matmul(a0, W1) + b1
a1 = threshold(u1)

u2 = tf.matmul(a1, W2)
# a2 = threshold(u2)
a2 = tf.nn.sigmoid(u2)

a2_fb = y

u1_fb = tf.matmul(a2_fb, tf.transpose(W2))
a1_fb = threshold(u1_fb)

y_t = threshold(tf.matmul(a1_fb, W2))
y_t_l = tf.nn.l2_loss(y-y_t)

u0_fb = tf.matmul(a1_fb, tf.transpose(W1))
a0_fb = threshold(u0_fb)



du0_mp = (a0_fb - a0)
du1_mp = (a1_fb - a1)
du2_mp = y - a2

loss = tf.reduce_sum(tf.square(a2 - y) / 2.0)

du0, du1 = tf.zeros((1,1)), tf.zeros((1,1))  #tf.gradients(loss, [u0, u1])

gr = [du0, du1]

# baseline
# 0.0243 test, 100 epoch, lrate 0.0001, adam, weight_factor = 1.0

    # simple mp
# 0.1235 test, 100 epoch, --//--

du0, du1, du2 = -du0_mp, -du1_mp, -du2_mp


dW0 = tf.matmul(tf.transpose(x), du0)
db0 = tf.reduce_sum(du0, 0)
dW1 = tf.matmul(tf.transpose(a0), du1)
db1 = tf.reduce_sum(du1, 0)
dW2 = tf.matmul(tf.transpose(a1), du2)


# opt = tf.train.GradientDescentOptimizer(learning_rate=0.0001)
opt = tf.train.AdamOptimizer(learning_rate=0.0001)
# opt = tf.train.RMSPropOptimizer(learning_rate=0.0001)

apply_grad_step = tf.group(
    opt.apply_gradients([
        (dW0, W0),
        (dW1, W1),
        (dW2, W2),
        # (db0, b0),
        # (db1, b1),
    ])
)

class_error_rate = tf.reduce_mean(
    tf.cast(tf.not_equal(tf.argmax(a2, 1), tf.argmax(y, 1)), tf.float32)
)


xv, yv = ds.next_train_batch()

sess = tf.Session()
sess.run(tf.global_variables_initializer())

epochs = 1000
train_metrics, test_metrics = (
    np.zeros((epochs, 2)),
    np.zeros((epochs, 2))
)

for epoch in range(epochs):
    for _ in range(ds.train_batches_num):
        xv, yv = ds.next_train_batch()

        u, a, loss_v, class_error_rate_v, gr0, y_t_l_v, _  = sess.run([
            [u0, u1],
            [a0, a1],
            loss,
            class_error_rate,
            gr,
            y_t_l,
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
            class_error_rate,
        ], {x: xtv, y: ytv})

        test_metrics[epoch] += (
            loss_t_v / ds.test_batches_num,
            class_error_rate_t_v / ds.test_batches_num
        )


    if epoch % 5 == 0:
        print("{}, train {:.4f} {:.4f}, test {:.4f} {:.4f} | {:.4f}".format(
            epoch,
            train_metrics[epoch][0],
            train_metrics[epoch][1],
            test_metrics[epoch][0],
            test_metrics[epoch][1],
            y_t_l_v,
        ))
