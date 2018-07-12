from poc.datasets import *
from poc.__scrap_tf__.common import *


class Layer(object):
    def __init__(s, input_size, layer_size, output_size, act, weight_factor):
        W, b = xavier_init(input_size, layer_size, weight_factor)

        s.act = act
        s.params = [
            tf.Variable(W),
            tf.Variable(b),
        ]

    def run(s, x, y):
        W, b = s.params
        s.u = tf.matmul(x, W)
        s.a = s.act(b + s.u)


def run_net(net, d):
    for li, l in enumerate(net):
        inp = d if li == 0 else net[li-1].a
        l.run(inp, None)


np.random.seed(11)
ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape

weight_factor = 1.0
threshold = 0.1

tf.set_random_seed(2)


x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")


layer_size = 500


ff_net = [
    Layer(input_size, layer_size, output_size, tf.nn.relu, weight_factor),
    Layer(layer_size, layer_size, layer_size, tf.nn.relu, weight_factor),
    Layer(layer_size, output_size, output_size, tf.nn.sigmoid, weight_factor)
]

fb_net = [
    Layer(output_size, layer_size, layer_size, tf.nn.relu, weight_factor),
    Layer(layer_size, layer_size, layer_size, tf.nn.relu, weight_factor),
    Layer(layer_size, input_size, input_size, tf.nn.sigmoid, weight_factor)
]

fb_net[1].params[0] = tf.transpose(ff_net[1].params[0])
fb_net[2].params[0] = tf.transpose(ff_net[0].params[0])


run_net(ff_net, x)
run_net(fb_net, y)


a_ff = tuple([l.a for l in ff_net])
a_fb = tuple([l.a for l in fb_net])

dW0 = tf.matmul(tf.transpose(x), a_fb[1] - a_ff[0])
dW1 = tf.matmul(tf.transpose(a_ff[0]), a_fb[0] - a_ff[1])
dW2 = tf.matmul(tf.transpose(a_ff[1]), y - a_ff[2])

opt = tf.train.GradientDescentOptimizer(learning_rate=0.0001)


class_error_rate = tf.reduce_mean(
    tf.cast(tf.not_equal(tf.argmax(a_ff[-1], 1), tf.argmax(y, 1)), tf.float32)
)

apply_grad_step = opt.apply_gradients([
    (-dW0, ff_net[0].params[0]),
    (-dW1, ff_net[1].params[0]),
    (-dW2, ff_net[2].params[0]),
])

sess = tf.Session()
sess.run(tf.global_variables_initializer())

epochs = 1000
print_freq = 1
train_metrics, test_metrics = (
    np.zeros((epochs, 1)),
    np.zeros((epochs, 1))
)

for epoch in range(100):
    for _ in range(ds.train_batches_num):
        xv, yv = ds.next_train_batch()

        a_ff_v, a_fb_v, class_error_rate_v, _ = sess.run((
            a_ff,
            a_fb,
            class_error_rate,
            apply_grad_step
        ), {
            x: xv,
            y: yv
        })

        train_metrics[epoch] += class_error_rate_v / ds.train_batches_num

    for _ in range(ds.test_batches_num):
        xtv, ytv = ds.next_test_batch()

        at_ff_v, at_fb_v, class_error_rate_t_v = sess.run((
            a_ff,
            a_fb,
            class_error_rate,
        ), {
            x: xtv,
            y: ytv
        })

        test_metrics[epoch] += class_error_rate_t_v / ds.test_batches_num

    if epoch % print_freq == 0:
        print("Epoch {}, train {:.4f}, test {:.4f}".format(
            epoch,
            class_error_rate_v,
            class_error_rate_t_v,
        ))

