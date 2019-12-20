
from util import *
from config import *
from common import *
from tasks import CountingTask


np.random.seed(12)

words = ["abcd", "bcda", "lacv"]

c = Config()

c.Ne = 100
c.Nu = 10
c.lambda_w = 10

c.N = c.Ne + c.Nu + c.Nu

c.T_max = 0.5
c.T_min = 0.0

c.upper_bound = 1.0

c.batch_size = 1

c.Nv = int(0.05 * c.Ne)
c.v_rate = 5.0
c.n_steps = 20000

c.eta_ip = 0.01
c.eta_stdp = 0.01

c.h_ip_mean = 0.2 * float(2 * c.Nu) / float(c.Ne)
c.h_ip_range = 0.01
c.h_ip = np.random.rand(c.N) * c.h_ip_range * 2 + c.h_ip_mean - c.h_ip_range

c.input_gain = 1.0
c.inh_gain = 10.0
c.I_alpha = 0.9

# task = CountingTask(words, c.Ne, c.Nv)

inp = gen_wmap(c, 0, c.Nu)
exc = gen_wmap(c, c.Nu, c.Nu + c.Ne)
out = gen_wmap(c, c.Nu + c.Ne, c.Nu + c.Ne + c.Nu)

s = Config()

s.W = random_pos_sparse_conn((c.N, c.N), c.lambda_w)
s.W[:, inp] = 0.0
s.W[out, :] = 0.0

# print_pressures(s.W, c)

s.M = np.where(s.W, True, False)

s.T = c.T_min + np.random.random((c.batch_size, c.N))*(c.T_max - c.T_min)
s.T[:, inp] = 0.0
s.T[:, out] = 0.0

synaptic_normalization(s.W)

silence_duration = 40


pattern = np.zeros((c.Nu, c.batch_size, c.Nu))
for ti in range(c.Nu):
    pattern[ti, :, ti] = 1.0

test = False

def run(pattern, c, test=False):
    x = np.zeros((c.batch_size, c.N))
    I = np.zeros((c.batch_size, 1))

    x_h = np.zeros((c.n_steps, c.batch_size, c.N))
    W_h = np.zeros((c.n_steps, c.N, c.N))
    T_h = np.zeros((c.n_steps, c.N))
    I_h = np.zeros((c.n_steps, 1))
    E_h = np.zeros((c.n_steps, 1))

    for step_id in range(c.n_steps):
        p_id = step_id % (pattern.shape[0] + silence_duration)

        if p_id < pattern.shape[0]:
            x[:, inp] = pattern[p_id]
            if not test:
                if p_id > 2:
                    x[:, out] = pattern[p_id]
                else:
                    x[:, out] = 0.0
        else:
            x[:, inp] = 0.0
            if not test:
                x[:, out] = 0.0

        R = np.dot(x, s.W)

        I = c.I_alpha * np.mean(x[:, exc], axis=1) + (1.0 - c.I_alpha) * I

        x_new = heavyside(R - s.T - c.inh_gain * I)

        if p_id < pattern.shape[0]:
            x_new[:, inp] = pattern[p_id]
            if not test:
                if p_id > 2:
                    x_new[:, out] = pattern[p_id]
                else:
                    x_new[:, out] = 0.0
        else:
            x_new[:, inp] = 0.0
            if not test:
                x_new[:, out] = 0.0

        s.T[:, exc] += c.eta_ip * (x_new[:, exc] - c.h_ip[exc])

        if step_id < c.n_steps - 1000:

            dW = stdp(x, x_new)

            s.W[s.M] += c.eta_stdp * dW[s.M]

            bound_weights(s.W, s.M, c)
            synaptic_normalization(s.W)

        x = x_new

        x_h[step_id] = x.copy()
        W_h[step_id] = s.W.copy()
        T_h[step_id] = s.T.copy()
        I_h[step_id] = np.sum(I)
        E_h[step_id] = np.linalg.norm(x[:, inp] - x[:, out])

    return x_h, W_h, T_h, I_h, E_h

# random_pattern = (np.random.random((10, c.batch_size, c.Nu)) < 0.1).astype(np.float)

x_h, W_h, T_h, I_h, E_h = run(pattern, c, test=False)

xt_h, Wt_h, Tt_h, It_h, Et_h = run(pattern, c.copy(eta_stdp=0.0, n_steps=5000), test=True)

