from poc.util import *

from scipy.sparse import issparse
import tensorflow as tf

def dense_to_sparse(tensor):
    tensor = tf.convert_to_tensor(tensor)
    indices = tf.where(tf.not_equal(tensor, tf.constant(0, tensor.dtype)))
    values = tf.gather_nd(tensor, indices)
    shape = tf.shape(tensor, out_type=tf.int64)
    return tf.SparseTensor(indices, values, shape)


def init_parameters(
    input_size,
    net_structure,
    weight_factor,
    lil_epsilon=1.0,
    big_epsilon=10.0
):
    params = []
    for fan_in, fan_out in zip((input_size,) + net_structure[:-1], net_structure):
        epsilon = lil_epsilon if (float(fan_in) + fan_out) / (fan_in * fan_out) > 0.05 else big_epsilon
        Winit, binit = sparse_xavier_init(fan_in, fan_out, const=weight_factor, epsilon=epsilon)
        Winit = Winit.T

        Wflat = tf.Variable(Winit.data)
        b = tf.Variable(binit)
        W = tf.SparseTensor(np.asarray((Winit.row, Winit.col)).T, Wflat, (fan_out, fan_in))

        params.append((W, b))
    return params




class Network(object):
    def __init__(
        s,
        input_size,
        net_structure,
        weight_factor,
        net_act,
        net_act_deriv,
        output_act,
        lil_epsilon=1.0,
        big_epsilon=10.0
    ):
        s.input_size = input_size
        s.net_structure = net_structure
        s.weight_factor = weight_factor

        s.net_act = net_act
        s.net_act_deriv = net_act_deriv

        s.output_act = output_act
        s.lil_epsilon = lil_epsilon
        s.big_epsilon = big_epsilon


        s.p = init_parameters(
            input_size,
            net_structure,
            weight_factor,
            lil_epsilon,
            big_epsilon
        )
        s.a = [None]*len(s.net_structure)
        s.u = [None]*len(s.net_structure)


    def run_net(s, x):
        for li, (W, b) in enumerate(params):
            inp = x if li == 0 else act[li - 1]

            u = tf.transpose(tf.sparse_tensor_dense_matmul(W, tf.transpose(inp))) + b
            a = s.net_act(u)

            s.a[li] = a
            s.u[li] = u

    def run_backprop(s):
        da = [None]*len(s.net_structure)
        dp = [None] * len(s.net_structure)
        for li in reversed(range(len(s.net_structure))):
            act = s.net_act if li < len(s.net_structure) - 1 else s.output_act

            da[li][:] = (
                deriv_loss(s.a[-1], y) if li == len(s.net_structure) - 1 else
                np.dot(s.da[li + 1], s.p[li + 1][0].T)
            )
            layer_input = x if li == 0 else s.a[li - 1]
            s.dp[li][0][:] = np.dot(layer_input.T, s.da[li] * act.deriv(s.m[li]))

            s.dp[li][1][:] = np.sum(s.da[li], 0).T


    #
    # METRIC_SIZE = 3
    # def calc_metrics(s, metrics, epoch, a, y, batches_num):
    #     metrics[epoch][0] += loss(a[-1], y) / batches_num
    #     metrics[epoch][1] += np.mean(np.not_equal(np.argmax(a[-1], 1), np.argmax(y, 1))) / batches_num
    #     if s.sparse:
    #         all_elem = s.a[0].shape[0] * s.a[0].shape[1]
    #         metrics[epoch][2] = (float(all_elem) - s.a[0].nnz)/ all_elem / batches_num
    #     else:
    #         metrics[epoch][2] += np.mean(np.equal(a[0], 0.0)) / batches_num
    #
    # def train(s, ds, epochs, print_freq=10):
    #     s.train_metrics, s.test_metrics = (
    #         np.zeros((epochs, Network.METRIC_SIZE)),
    #         np.zeros((epochs, Network.METRIC_SIZE))
    #     )
    #
    #     for epoch in xrange(epochs):
    #         for _ in xrange(ds.train_batches_num):
    #             x, y = ds.next_train_batch()
    #             s.run_net(x, s.m, s.a)
    #
    #             s.run_backprop(x, y)
    #
    #             s.calc_metrics(s.train_metrics, epoch, s.a, y, ds.train_batches_num)
    #
    #         for _ in xrange(ds.test_batches_num):
    #             xt, yt = ds.next_test_batch()
    #             s.run_net(xt, s.mt, s.at)
    #             s.calc_metrics(s.test_metrics, epoch, s.at, yt, ds.test_batches_num)
    #
    #         if epoch % print_freq == 0:
    #             print("Epoch {}, train {:.4f} {:.4f} {:.4f}, test {:.4f} {:.4f} {:.4f}".format(
    #                 epoch,
    #                 s.train_metrics[epoch][0],
    #                 s.train_metrics[epoch][1],
    #                 s.train_metrics[epoch][2],
    #                 s.test_metrics[epoch][0],
    #                 s.test_metrics[epoch][1],
    #                 s.test_metrics[epoch][2],
    #             ))
    #
    #
