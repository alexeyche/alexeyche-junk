from poc.datasets import *
from poc.__scrap_tf__.common import *

def relu_deriv(x):
    a = tf.nn.relu(x)
    da = tf.where(a > 0.0, tf.ones(tf.shape(a)), tf.zeros(tf.shape(a)))
    return da


def sigmoid_deriv(x):
    v = tf.sigmoid(x)
    return v * (1.0 - v)

def ltd(a_ff, a_fb):
    ltd = tf.zeros(tf.shape(a_ff))
    ltd = tf.where(a_fb < 1e-10, a_ff, ltd)
    return ltd


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
        s.u = tf.matmul(x, W) + b
        s.a = s.act(s.u)


def run_net(net, d):
    for li, l in enumerate(net):
        inp = d if li == 0 else net[li-1].a
        l.run(inp, None)


seed = 12
np.random.seed(seed)
tf.set_random_seed(seed)

# ds = ToyDataset()
# ds = XorDataset()
# ds = XorDatasetSmall()
ds = MNISTDataset()

(_, input_size), (_, output_size) = ds.train_shape

weight_factor = 0.1



x = tf.placeholder(tf.float32, shape=(None, input_size), name="x")
y = tf.placeholder(tf.float32, shape=(None, output_size), name="y")


layer_size = 500




ff_net = [
    Layer(input_size, layer_size, output_size, tf.nn.relu, weight_factor),
    # Layer(layer_size, layer_size, layer_size, tf.nn.relu, weight_factor),
    Layer(layer_size, output_size, output_size, tf.nn.relu, weight_factor)
]

fb_net = [
    Layer(output_size, layer_size, layer_size, tf.nn.relu, weight_factor),
    # Layer(layer_size, layer_size, layer_size, tf.nn.relu, weight_factor),
    Layer(layer_size, input_size, input_size, tf.nn.relu, weight_factor)
]

for ff_l, fb_l in zip(ff_net, reversed(fb_net)):
    fb_l.params[0] = tf.transpose(ff_l.params[0])
    fb_l.params[1] = tf.zeros(fb_l.params[1].get_shape())

run_net(ff_net, x)
run_net(fb_net, y)

net_a_ff = [l.a for l in ff_net]
net_a_fb = [l.a for l in fb_net]

dus = []
grads_and_vars = []
deb = []
for li, (inp, a_ff, a_fb) in enumerate(zip([x] + net_a_ff[:-1], net_a_ff, reversed([y] + net_a_fb[:-1]))):
    if li < len(ff_net)-1:
        du = (
            (a_fb - ltd(a_ff, a_fb)) * relu_deriv(ff_net[li].u)
        )

        # du = (
        #     (ltd(a_ff, a_fb) - tf.nn.relu(tf.matmul(y, tf.transpose(ff_net[li + 1].params[0])))) * relu_deriv(ff_net[li].u)
        # )

        # de = (net_a_ff[li+1] - y) #* relu_deriv(ff_net[li+1].u)
        # du = (
        #      tf.matmul(de, tf.transpose(ff_net[li + 1].params[0])) * relu_deriv(ff_net[li].u)
        # )
    else:
        # du = (a_fb - a_ff) * sigmoid_deriv(ff_net[li].u)
        du = (a_fb - a_ff) * relu_deriv(ff_net[li].u)

    dus.append(du)

    dW = tf.matmul(tf.transpose(inp), du)
    db = tf.reduce_sum(du, 0)

    grads_and_vars.append((-dW, ff_net[li].params[0]))
    grads_and_vars.append((-db, ff_net[li].params[1]))



opt = tf.train.GradientDescentOptimizer(learning_rate=0.0001)
# opt = tf.train.AdamOptimizer(learning_rate=0.001)

apply_grad_step = opt.apply_gradients(grads_and_vars)


class_error_rate = tf.reduce_mean(
    tf.cast(tf.not_equal(tf.argmax(net_a_ff[-1], 1), tf.argmax(y, 1)), tf.float32)
)

loss = tf.square(ff_net[-1].a - y) / 2.0

real_grad_step = opt.minimize(loss)

sess = tf.Session()
sess.run(tf.global_variables_initializer())

# xv, yv = ds.next_train_batch()
#
# loss = tf.square(ff_net[-1].a - y) / 2.0
#
# g_and_v = tf.gradients(loss, [
#     ff_net[0].params[0],
#     ff_net[1].params[0],
# ])
#
# du_r = tf.gradients(loss, [ff_net[0].u, ff_net[1].u])
#
# g_and_v_bp, dus_v, du_r_v, a0v, a1v, deb_v, g_and_v_mp = sess.run([
#     g_and_v,
#     dus,
#     du_r,
#     net_a_ff,
#     net_a_fb,
#     deb,
#     grads_and_vars,
# ], {x: xv, y: yv})




epochs = 200
print_freq = 5
train_metrics, test_metrics = (
    np.zeros((epochs, 1)),
    np.zeros((epochs, 1))
)


for epoch in range(epochs):
    for _ in range(ds.train_batches_num):
        xv, yv = ds.next_train_batch()

        a_ff_v, a_fb_v, class_error_rate_v, _ = sess.run((
            net_a_ff,
            net_a_fb,
            class_error_rate,
            apply_grad_step,
        ), {
            x: xv,
            y: yv
        })

        train_metrics[epoch] += (
            class_error_rate_v / ds.train_batches_num
        )

    for _ in range(ds.test_batches_num):
        xtv, ytv = ds.next_test_batch()

        at_ff_v, at_fb_v, class_error_rate_t_v = sess.run((
            net_a_ff,
            net_a_fb,
            class_error_rate
        ), {
            x: xtv,
            y: ytv
        })

        test_metrics[epoch] += (
            class_error_rate_t_v / ds.test_batches_num
        )

    if epoch % print_freq == 0:
        print("Epoch {}, train {:.4f}, test {:.4f}".format(
            epoch,
            train_metrics[epoch][0],
            test_metrics[epoch][0],
        ))

