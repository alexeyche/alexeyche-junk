
import numpy as np
from sklearn.neural_network._stochastic_optimizers import *
from poc.datasets import *
from poc.util import *


class Sigmoid(object):
    def __call__(self, x):
        return 1.0/(1.0 + np.exp(-x))

    def deriv(self, x):
        v = self(x)
        return v * (1.0 - v)

class Relu(object):
    def __call__(self, x):
        return np.maximum(x, 0.0)
        
    def deriv(self, x):
        if isinstance(x, float):
            return 1.0 if x > 0.0 else 0.0
        dadx = np.zeros(x.shape)
        dadx[np.where(x > 0.0)] = 1.0
        return dadx



def init_parameters(
    input_size,
    batch_size,
    net_structure,
    weight_factor
):
    params, deriv_params, deriv_layer = [], [], []
    for fan_in, fan_out in zip((input_size,) + net_structure[:-1], net_structure):
        params.append(xavier_init(fan_in, fan_out, weight_factor))

        deriv_layer.append(np.zeros((batch_size, fan_out,)))
        deriv_params.append((
            np.zeros((fan_in, fan_out)),
            np.zeros((fan_out,))
        ))
    return params, deriv_params, deriv_layer

def init_network(net_structure, batch_size, test_batch_size):
    membrane = [np.zeros((batch_size, l)) for l in net_structure]
    activity = [np.zeros((batch_size, l)) for l in net_structure]

    test_membrane = [np.zeros((test_batch_size, l)) for l in net_structure]
    test_activity = [np.zeros((test_batch_size, l)) for l in net_structure]

    return membrane, activity, test_membrane, test_activity



np.random.seed(11)
ds = ToyDataset()

(_, input_size), (_, output_size) = ds.train_shape

input_size=input_size
batch_size=ds.train_batch_size
test_batch_size=ds.test_batch_size
net_structure=(100, output_size)
fb_net_structure = net_structure[:-1] + (input_size,)
weight_factor=1.0
net_act=Relu()
output_act=Sigmoid()
deriv_loss = lambda x, y: x - y
loss = lambda x, y: np.linalg.norm(x - y)/2.0

p, dp, da = init_parameters(
    input_size,
    batch_size,
    net_structure,
    weight_factor
)

p_fb, dp_fb, da_fb = init_parameters(
    output_size,
    batch_size,
    fb_net_structure,
    weight_factor
)


m, a, mt, at = init_network(net_structure, batch_size, test_batch_size)
m_fb, a_fb, mt_fb, at_fb = init_network(fb_net_structure, batch_size, test_batch_size)

# opt = SGDOptimizer(flatten(p), learning_rate_init=0.0001)
opt = AdamOptimizer(flatten(p), learning_rate_init=0.001)


def run_backprop(x, y):
    for li in reversed(range(len(net_structure))):
        act = net_act if li < len(net_structure) - 1 else output_act
        layer_input = x if li == 0 else a[li - 1]

        da[li][:] = (
            deriv_loss(a[-1], y) if li == len(net_structure) - 1 else
            np.dot(da[li + 1], p[li + 1][0].T)
        )
        

        dp[li][0][:] = np.dot(layer_input.T, da[li] * act.deriv(m[li]))
        dp[li][1][:] = np.sum(da[li], 0).T

    opt.update_params(flatten(dp))


def run_matchprop(x, y):
    for li in reversed(range(len(net_structure))):
        act = net_act if li < len(net_structure) - 1 else output_act
        layer_input = x if li == 0 else a[li - 1]

        if li == len(net_structure) - 1:
            da[li][:] = a[li] - y
        else:
            da[li][:] = a[li] - a_fb[len(net_structure)-2-li]

        dp[li][0][:] = np.dot(layer_input.T, da[li] * act.deriv(m[li]))
        dp[li][1][:] = np.sum(da[li], 0).T

    opt.update_params(flatten(dp))


def run_ch(a_neg, a_pos):
    for li, (a_n, a_p) in enumerate(zip(a_neg, a_pos)):
        if li == len(net_structure) - 1:
            da[li][:] = a[li] - y
        else:
            da[li][:] = a[li] - a_fb[len(net_structure)-2-li]

        dp[li][0][:] = np.dot(layer_input.T, da[li] * act.deriv(m[li]))
        dp[li][1][:] = np.sum(da[li], 0).T

    opt.update_params(flatten(dp))


def run_net(x, p, mem, act):
    for li, (W, b) in enumerate(p):
        inp = x if li == 0 else act[li-1]
        f = net_act if li < len(net_structure)-1 else output_act

        mem[li][:] = np.dot(inp, W) + b
        act[li] = f(mem[li][:])


def run_rec_net(x, y, p, mem, act, lam=0.1, positive=False):
    for _ in xrange(5):
        for li, (W, b) in enumerate(p):
            inp = x if li == 0 else act[li-1]
            f = net_act if li < len(net_structure)-1 else output_act

            if li == len(net_structure)-1:
                mem[li][:] = np.dot(inp, W) + b
                if positive:
                    act[li] = y
                else:
                    new_a = f(mem[li][:])
                    # print "\t", np.linalg.norm(new_a - act[li])
                    act[li] = new_a
            else:
                Wnext, _ = p[li+1]
                anext = act[li+1]
                mem[li][:] = np.dot(inp, W) + b + lam * np.dot(anext, Wnext.T)
                new_a = f(mem[li][:])
                
                # print np.linalg.norm(new_a - act[li])
                act[li] = new_a


METRIC_SIZE = 3
def calc_metrics(metrics, epoch, aw, y, batches_num):
    metrics[epoch][0] += loss(aw[-1], y) / batches_num
    metrics[epoch][1] += np.mean(np.not_equal(np.argmax(aw[-1], 1), np.argmax(y, 1))) / batches_num
    metrics[epoch][2] += np.linalg.norm(da[0]) / batches_num



epochs = 1
train_metrics, test_metrics = (
    np.zeros((epochs, METRIC_SIZE)),
    np.zeros((epochs, METRIC_SIZE))
)

p_fb[1] = (p[0][0].T, p_fb[1][1])

print_freq = 100
for epoch in xrange(epochs):
    for _ in xrange(ds.train_batches_num):
        x, y = ds.next_train_batch()
        # run_net(x, p, m, a)
        
        # run_backprop(x, y)
        # dp_bp = [(dW.copy(), db.copy()) for dW, db in dp]
        # da_bp = [dda.copy() for dda in da]

        # run_net(y, p_fb, m_fb, a_fb)
        # run_matchprop(x, y)
        
        run_rec_net(x, y, p, m, a, lam=0.1, positive=False)
        a_neg = a.copy()

        run_rec_net(x, y, p, m, a, lam=0.1, positive=True)
        
        run_ch(a_neg, a)

        calc_metrics(train_metrics, epoch, a, y, ds.train_batches_num)

    # for _ in xrange(ds.test_batches_num):
    #     xt, yt = ds.next_test_batch()
    #     run_net(xt, p, mt, at)
    #     calc_metrics(test_metrics, epoch, at, yt, ds.test_batches_num)

    p_fb[1] = (p[0][0].T, p_fb[1][1])

    if epoch % print_freq == 0:
        print("Epoch {}, train {:.4f} {:.4f} {:.4f}, test {:.4f} {:.4f} {:.4f}".format(
            epoch,
            train_metrics[epoch][0],
            train_metrics[epoch][1],
            train_metrics[epoch][2],
            test_metrics[epoch][0],
            test_metrics[epoch][1],
            test_metrics[epoch][2],
        ))


