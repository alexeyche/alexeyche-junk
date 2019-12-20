

from util import *
from config import *
from common import *
from tasks import CountingTask


c = Config()

c.N = 10
c.batch_size = 1
c.n_steps = 500

W = 0.1 * np.random.randn(c.N, c.N)
synaptic_normalization(W)




pattern = np.zeros((c.N, c.batch_size, c.N))

for ti in range(c.N):
    pattern[ti, :, ti] = 1.0

silence_duration = 50


def run(test=False):
    x = np.zeros((c.batch_size, c.N))

    x_h = np.zeros((c.n_steps, c.batch_size, c.N))
    y_h = np.zeros((c.n_steps, c.batch_size, c.N))
    R_h = np.zeros((c.n_steps, c.batch_size, c.N))
    W_h = np.zeros((c.n_steps, c.N, c.N))

    for step_id in range(c.n_steps):
        p_id = step_id % (pattern.shape[0] + silence_duration)
        pt_id = (step_id+5) % (pattern.shape[0] + silence_duration)


        if p_id < pattern.shape[0]:
            x[:, :] = pattern[p_id]
        else:
            x[:, :] = 0.0

        R = np.dot(x, W)

        y = heavyside(R)

        if not test:
            if pt_id < pattern.shape[0]:
                y[:, :] = pattern[pt_id]
            else:
                y[:, :] = 0.0

            dW = stdp(x, y)

            W += 0.01 * dW

        x_h[step_id] = x.copy()
        y_h[step_id] = y.copy()
        R_h[step_id] = R.copy()
        W_h[step_id] = W.copy()

    return x_h, y_h, R_h, W_h


x_h, y_h, R_h, W_h = run(test=False)
xt_h, yt_h, Rt_h, Wt_h = run(test=True)

