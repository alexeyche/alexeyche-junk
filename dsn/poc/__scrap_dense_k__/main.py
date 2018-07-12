
from poc.datasets import *
from poc.util import *
from sklearn.datasets.samples_generator import make_blobs


def norm(W):
    delim = np.linalg.norm(W, 2, axis=0, keepdims=True)
    delim[np.where(delim == 0)] = 1.0
    return W / delim

def act(u, K):
    a = np.zeros(u.shape)
    batch_size = u.shape[0]

    bidx = np.arange(0, batch_size)
    ind = np.argpartition(u, -K, axis=1)[:, -K:]

    a[np.expand_dims(bidx, 1), ind] = 1.0
    a[np.where(np.abs(u) < 1e-10)] = 0.0
    return a


class Layer(object):
    def __init__(s, batch_size, input_size, layer_size, output_size, epochs, weight_factor, sparsity, K, lrate):
        s.batch_size = batch_size
        s.input_size = input_size
        s.layer_size = layer_size
        s.output_size = output_size
        s.weight_factor = weight_factor
        s.sparsity = sparsity
        s.K = K
        s.lrate = lrate

        s.W, _ = sparse_xavier_init(input_size, layer_size, const=weight_factor, p=1.0-sparsity)
        s.W = np.asarray(s.W.todense())

        # s.W, _ = xavier_init(input_size, layer_size, const=weight_factor)

        s.Winit = s.W.copy()
        
        s.uh = np.zeros((epochs, batch_size, layer_size))
        s.ah = np.zeros((epochs, batch_size, layer_size))
        s.dah = np.zeros((epochs, 1))

        s.act = lambda x: act(x, K)

    def run(s, epoch, x):
        s.u = np.dot(x, s.W)
        s.a = s.act(s.u)

        s.dW = np.dot(x.T, s.a) / s.batch_size
        s.W += s.lrate * s.dW
        # s.W = norm(s.W)

        s.uh[epoch] = s.u.copy()
        s.ah[epoch] = s.a.copy()
        
        if epoch > 0:
            s.dah[epoch] = np.mean(np.not_equal(s.ah[epoch], s.ah[epoch-1]))


input_size = 5
output_size = 5
weight_factor = 0.1
layer_size = 100

sparsity = 0.95
batch_size = 5
K = layer_size // 20
dt = 0.01
reg = 0.0

lrate = 0.1

epochs = 200

net = [
    Layer(
        batch_size, 
        input_size, 
        layer_size, 
        layer_size, 
        epochs, 
        weight_factor, 
        sparsity, 
        K,
        lrate
    ),
    Layer(
        batch_size, 
        layer_size, 
        layer_size, 
        layer_size, 
        epochs, 
        weight_factor, 
        sparsity, 
        K,
        lrate
    )
]

np.random.seed(4)


x = np.asarray([
    [1.0, 0.0, 0.0, 0.0, 0.0],
    [0.0, 1.0, 0.0, 0.0, 0.0],
    [0.0, 0.0, 1.0, 0.0, 0.0],
    [0.0, 0.0, 0.0, 1.0, 0.0],
    [0.0, 0.0, 0.0, 0.0, 1.0],
])


for epoch in range(epochs):
    for li, l in enumerate(net):
        inp = x if li == 0 else net[li-1].a
            
        l.run(epoch, inp)


    print("{} {:.4f} {:.4f}, {:.4f} {:.4f}".format(
        epoch,
        np.linalg.norm(net[0].dW),
        np.linalg.norm(net[1].dW),
        net[0].dah[epoch, 0],
        net[1].dah[epoch, 0],
    ))
