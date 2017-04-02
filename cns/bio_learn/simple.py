
from matplotlib import pyplot as plt
from matplotlib.pyplot import cm
import numpy as np

from common import Sigmoid, Linear, get_oja_deriv, norm, Learning

act = Linear()

lrate = 1e-01
x = 0.5
T = 1
lrule = Learning.FA

epochs = 1

y_t = 0.1

W0 = 1.0
W1 = 1.0
B0 = 1.0
slice_size = 25

W0a = np.linspace(0, 1.0, slice_size)
W1a = np.linspace(0, 1.0, slice_size)

W0res = np.zeros((slice_size, slice_size))
W1res = np.zeros((slice_size, slice_size))
dW0res = np.zeros((slice_size, slice_size))
dW1res = np.zeros((slice_size, slice_size))
error_res = np.zeros((slice_size, slice_size))

for ri, W0 in enumerate(W0a):
    for ci, W1 in enumerate(W1a):
        for epoch in xrange(epochs):

            a0 = x * W0; h0 = act(a0)
            a1 = h0 * W1; y = act(a1)

            e = y - y_t
            error = (y - y_t) ** 2.0

            if lrule == Learning.BP or lrule == Learning.FA:
                if lrule == Learning.BP:
                    dh0 = W1 * e * act.deriv(a0)
                else:
                    dh0 = e * B0 * act.deriv(a0)

                dW0 = - x * dh0
                dW1 = - h0 * e

            W0 += lrate * dW0
            W1 += lrate * dW1

            print "Epoch {}, error {}".format(epoch, error)

            W0res[ri, ci] = W0
            W1res[ri, ci] = W1
            dW0res[ri, ci] = dW0
            dW1res[ri, ci] = dW1
            error_res[ri, ci] = error

plot = plt.figure()
plt.quiver(
    W0res, W1res, dW0res, dW0res,
    error_res,
    cmap=cm.seismic,     # colour map
    headlength=7, headwidth=5.0)        # length of the arrows

plt.colorbar()                      # add colour bar on the right
plt.show(plot)