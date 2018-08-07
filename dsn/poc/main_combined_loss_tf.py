
from poc.datasets import *
from poc.util import *

import tensorflow as tf

np.random.seed(12)
tf.set_random_seed(12)

ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape

x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")

learning = tf.placeholder(tf.bool, shape=(), name="learning")

layer_size = 100

# W0v = random_pos_orth((input_size, layer_size))
# W1v = random_pos_orth((layer_size, output_size_emb))

W0v = np.random.random((input_size, layer_size)).astype(np.float32) - 0.5
W1v = np.random.random((layer_size, output_size)).astype(np.float32) - 0.5
R1v = np.random.random((output_size, layer_size)).astype(np.float32) - 0.5


# W0v = np.ones((input_size, layer_size))
W0 = tf.Variable(W0v)
W1 = tf.Variable(W1v)
R1 = tf.Variable(R1v)


a0 = tf.nn.relu(tf.matmul(x, W0))
a1 = tf.nn.sigmoid(tf.matmul(a0, W1))

a1_target = tf.cond(learning, lambda: y, lambda: a1)
a0_target = tf.nn.relu(tf.matmul(a1_target, R1))

y_hat = a1

loss0 = tf.nn.l2_loss(a0 - a0_target)
loss1 = tf.nn.l2_loss(a1 - a1_target)

loss = loss0 + loss1

# dW0_inner = 2.0 * tf.tile(tf.reduce_sum(W0, axis=1, keepdims=True), (1, layer_size))
# dW0_c = 2.0 * tf.matmul(dW0_inner, v) / layer_size

class_error_rate = tf.reduce_mean(
    tf.cast(tf.not_equal(tf.argmax(y_hat, 1), tf.argmax(y, 1)), tf.float32)
)


dW0 = -tf.matmul(tf.transpose(x), a0_target - a0)
dW1 = -tf.matmul(tf.transpose(a0), a1_target - a1)
dR1 = -tf.matmul(tf.transpose(a1_target), a0_target - a0)

opt = tf.train.GradientDescentOptimizer(learning_rate=0.0001)

apply_grad_step = tf.group(
    opt.apply_gradients((
        (dW0, W0),
        (dW1, W1),
        (dR1, R1),
    )),
)

sess = tf.Session()
sess.run(tf.global_variables_initializer())


epochs = 100
train_metrics, test_metrics = (
    np.zeros((epochs, 3)),
    np.zeros((epochs, 3))
)


for epoch in range(epochs):

    for _ in range(ds.train_batches_num):
        xv, yv = ds.next_train_batch()

        loss_v, class_error_rate_v, av, _ = sess.run([
            [loss0, loss1],
            class_error_rate,
            [a0, a1],
            apply_grad_step,
        ], {x: xv, y: yv, learning: epoch in flatten([range(10, 20), range(30, 40), range(50, 60), range(70, 100)])})

        train_metrics[epoch] += (
            loss_v[0] / ds.train_batches_num,
            loss_v[1] / ds.train_batches_num,
            class_error_rate_v / ds.train_batches_num,
        )

    for _ in range(ds.test_batches_num):
        xtv, ytv = ds.next_test_batch()

        loss_t_v, class_error_rate_t_v, atv = sess.run([
            [loss0, loss1],
            class_error_rate,
            [a0, a1]
        ], {x: xtv, y: ytv, learning: False})

        test_metrics[epoch] += (
            loss_t_v[0] / ds.test_batches_num,
            loss_t_v[1] / ds.test_batches_num,
            class_error_rate_t_v / ds.test_batches_num,
        )

    if epoch % 5 == 0:
        print("{}, train {:.4f} {:.4f} {:.4f}, test {:.4f} {:.4f} {:.4f}".format(
            epoch,
            train_metrics[epoch][0],
            train_metrics[epoch][1],
            train_metrics[epoch][2],
            test_metrics[epoch][0],
            test_metrics[epoch][1],
            test_metrics[epoch][2],
        ))



# shm(2.0*np.tile(np.sum(W0v, 1), (layer_size, 1)).T, dW0v)