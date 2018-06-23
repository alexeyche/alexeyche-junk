

import numpy as np

from poc.datasets import *
from poc.util import *
from poc.act import *


from sklearn.neural_network._stochastic_optimizers import AdamOptimizer
from sklearn.neural_network._base import binary_log_loss

np.random.seed(11)
ds = XorDataset()


(_, input_size), (_, output_size) = ds.train_shape
batch_size = ds.train_batch_size
test_batch_size = ds.test_batch_size

weight_factor = 1.0
net_structure = (100, output_size)


net_act = Relu()
output_act = Sigmoid()

loss = lambda x, y: np.linalg.norm(x - y)/2.0
deriv_loss = lambda x, y: x - y


params, deriv_params, deriv_layer = [], [], []
for fan_in, fan_out in zip((input_size,) + net_structure[:-1], net_structure):
    params.append(xavier_init(fan_in, fan_out, weight_factor))
    deriv_params.append((
        np.zeros((fan_in, fan_out)),
        np.zeros((fan_out,))
    ))
    deriv_layer.append(np.zeros((batch_size, fan_out,)))


params_size = np.sum([len(p) for p in params])

membrane = [np.zeros((batch_size, l)) for l in net_structure]
activity = [np.zeros((batch_size, l)) for l in net_structure]

test_membrane = [np.zeros((test_batch_size, l)) for l in net_structure]
test_activity = [np.zeros((test_batch_size, l)) for l in net_structure]


opt = AdamOptimizer([pp for pp in p for p in params], 0.001, 0.001)


def run_net(x, mem, act):
    for li, (W, b) in enumerate(params):
        inp = x if li == 0 else act[li-1]
        f = net_act if li < len(net_structure)-1 else output_act

        mem[li][:] = np.dot(inp, W) + b
        act[li][:] = f(mem[li][:])


epochs = 3000
train_metrics, test_metrics = np.zeros((epochs, 2)), np.zeros((epochs, 2))
for epoch in xrange(epochs):
    for _ in xrange(ds.train_batches_num):
        x, y = ds.next_train_batch()

        run_net(x, membrane, activity)


        for li in reversed(range(len(net_structure))):
            act = net_act if li < len(net_structure)-1 else output_act

            deriv_layer[li][:] = (
                deriv_loss(activity[-1], y) if li == len(net_structure)-1 else
                np.dot(deriv_layer[li + 1], params[li + 1][0].T)
            )
            layer_input = x if li == 0 else activity[li-1]

            deriv_params[li][0][:] = np.dot(layer_input.T, deriv_layer[li] * act.deriv(membrane[li]))
            deriv_params[li][1][:] = np.sum(deriv_layer[li], 0)

        # opt.update(*[pp for pp in p for p in deriv_params])
        opt.update_params([pp for pp in p for p in deriv_params])

        train_metrics[epoch][0] += loss(activity[-1], y) / ds.train_batches_num
        train_metrics[epoch][1] += np.mean(np.not_equal(np.argmax(activity[-1], 1), np.argmax(y, 1))) / ds.train_batches_num


    for _ in xrange(ds.test_batches_num):
        x, y = ds.next_test_batch()

        run_net(x, test_membrane, test_activity)

        test_metrics[epoch][0] += loss(test_activity[-1], y) / ds.test_batches_num
        test_metrics[epoch][1] += np.mean(np.not_equal(np.argmax(test_activity[-1], 1), np.argmax(y, 1))) / ds.test_batches_num

    if epoch % 10 == 0:
        print "Epoch {}, train {:.4f} {:.4f}, test {:.4f} {:.4f}".format(
            epoch,
            train_metrics[epoch][0],
            train_metrics[epoch][1],
            test_metrics[epoch][0],
            test_metrics[epoch][1]
        )




















