from poc.util import *

from scipy.sparse import issparse

loss = lambda x, y: np.linalg.norm(x - y)/2.0
deriv_loss = lambda x, y: x - y

from poc.__scrap__.opt import *

def init_parameters(
    input_size,
    batch_size,
    net_structure,
    weight_factor,
    sparse=False,
    lil_epsilon=1.0,
    big_epsilon=10.0
):
    params, deriv_params, deriv_layer = [], [], []
    for fan_in, fan_out in zip((input_size,) + net_structure[:-1], net_structure):
        if sparse:
            epsilon = lil_epsilon if (float(fan_in) + fan_out) / (fan_in * fan_out) > 0.05 else big_epsilon
            params.append(sparse_xavier_init(fan_in, fan_out, weight_factor, epsilon=epsilon))
        else:
            params.append(xavier_init(fan_in, fan_out, weight_factor))

        if sparse:
            deriv_params.append((
                csr_matrix((fan_in, fan_out)),
                csr_matrix((fan_out,1,))
            ))
            deriv_layer.append(csr_matrix((batch_size, fan_out,)))
        else:
            deriv_params.append((
                np.zeros((fan_in, fan_out)),
                np.zeros((fan_out,))
            ))
            deriv_layer.append(np.zeros((batch_size, fan_out,)))

    return params, deriv_params, deriv_layer



def init_network(net_structure, batch_size, test_batch_size):
    membrane = [np.zeros((batch_size, l)) for l in net_structure]
    activity = [np.zeros((batch_size, l)) for l in net_structure]

    test_membrane = [np.zeros((test_batch_size, l)) for l in net_structure]
    test_activity = [np.zeros((test_batch_size, l)) for l in net_structure]

    return membrane, activity, test_membrane, test_activity


class Network(object):
    def __init__(
        s,
        input_size,
        batch_size,
        test_batch_size,
        net_structure,
        weight_factor,
        net_act,
        output_act,
        lrate,
        sparse=False,
        lil_epsilon=1.0,
        big_epsilon=10.0
    ):
        s.input_size = input_size
        s.batch_size = batch_size
        s.test_batch_size = test_batch_size
        s.net_structure = net_structure
        s.weight_factor = weight_factor
        s.net_act = net_act
        s.output_act = output_act
        s.sparse = sparse
        s.lil_epsilon = lil_epsilon
        s.big_epsilon = big_epsilon


        s.p, s.dp, s.da = init_parameters(
            input_size,
            batch_size,
            net_structure,
            weight_factor,
            sparse,
            lil_epsilon,
            big_epsilon
        )

        s.m, s.a, s.mt, s.at = init_network(net_structure, batch_size, test_batch_size)
        if s.sparse:
            s.opt = AdamOptSparse(s.flat_p, lrate)
        else:
            s.opt = AdamOpt(s.flat_p, lrate)

    @property
    def flat_p(s):
        return [pp  for param in s.p for pp in param]

    @property
    def flat_dp(s):
        return [pp for param in s.dp for pp in param]

    def run_net(s, x, mem, act):
        for li, (W, b) in enumerate(s.p):
            inp = x if li == 0 else act[li-1]
            f = s.net_act if li < len(s.net_structure)-1 else s.output_act

            if s.sparse and not issparse(inp):
                inp = csr_matrix(inp)

            mem[li][:] = np.dot(inp, W) + b
            act[li] = f(mem[li][:])

    def run_backprop(s, x, y):
        for li in reversed(range(len(s.net_structure))):
            act = s.net_act if li < len(s.net_structure) - 1 else s.output_act

            s.da[li][:] = (
                deriv_loss(s.a[-1], y) if li == len(s.net_structure) - 1 else
                np.dot(s.da[li + 1], s.p[li + 1][0].T)
            )
            layer_input = x if li == 0 else s.a[li - 1]

            if s.sparse:
                if not issparse(layer_input):
                    layer_input = csr_matrix(layer_input)
                s.dp[li][0][:] = np.dot(layer_input.T, s.da[li].multiply(act.deriv(s.m[li])))
            else:
                s.dp[li][0][:] = np.dot(layer_input.T, s.da[li] * act.deriv(s.m[li]))

            s.dp[li][1][:] = np.sum(s.da[li], 0).T

        s.opt.update_params(s.flat_dp)

    METRIC_SIZE = 3
    def calc_metrics(s, metrics, epoch, a, y, batches_num):
        metrics[epoch][0] += loss(a[-1], y) / batches_num
        metrics[epoch][1] += np.mean(np.not_equal(np.argmax(a[-1], 1), np.argmax(y, 1))) / batches_num
        if s.sparse:
            all_elem = s.a[0].shape[0] * s.a[0].shape[1]
            metrics[epoch][2] = (float(all_elem) - s.a[0].nnz)/ all_elem / batches_num
        else:
            metrics[epoch][2] += np.mean(np.equal(a[0], 0.0)) / batches_num

    def train(s, ds, epochs, print_freq=10):
        s.train_metrics, s.test_metrics = (
            np.zeros((epochs, Network.METRIC_SIZE)),
            np.zeros((epochs, Network.METRIC_SIZE))
        )

        for epoch in xrange(epochs):
            for _ in xrange(ds.train_batches_num):
                x, y = ds.next_train_batch()
                s.run_net(x, s.m, s.a)

                s.run_backprop(x, y)

                s.calc_metrics(s.train_metrics, epoch, s.a, y, ds.train_batches_num)

            for _ in xrange(ds.test_batches_num):
                xt, yt = ds.next_test_batch()
                s.run_net(xt, s.mt, s.at)
                s.calc_metrics(s.test_metrics, epoch, s.at, yt, ds.test_batches_num)

            if epoch % print_freq == 0:
                print("Epoch {}, train {:.4f} {:.4f} {:.4f}, test {:.4f} {:.4f} {:.4f}".format(
                    epoch,
                    s.train_metrics[epoch][0],
                    s.train_metrics[epoch][1],
                    s.train_metrics[epoch][2],
                    s.test_metrics[epoch][0],
                    s.test_metrics[epoch][1],
                    s.test_metrics[epoch][2],
                ))


