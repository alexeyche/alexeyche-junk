
from util import *
from config import *
from common import *
from tasks import CountingTask


np.random.seed(10)

words = ["abcd", "bcda", "lacv"]

c = Config()

c.Ne = 100
c.Ni = int(0.2 * c.Ne)
c.Nu = 10
c.lambda_w = 10

c.Te_max = 0.5
c.Te_min = 0.0
c.Ti_max = 0.35
c.Ti_min = 0.0

c.upper_bound = 1.0

c.batch_size = 1

c.Nv = int(0.05 * c.Ne)
c.v_rate = 5.0
c.n_steps = 100000

c.eta_ip = 0.01
c.eta_stdp = 0.01

c.h_ip_mean = 0.5 * float(2 * c.Nu) / float(c.Ne)
c.h_ip_range = 0.01
c.h_ip = np.random.rand(c.Ne) * c.h_ip_range * 2 + c.h_ip_mean - c.h_ip_range

c.input_gain = 1.0

task = CountingTask(words, c.Ne, c.Nv)


s = Config()
s.Wei = np.random.random((c.Ne, c.Ni))
s.Wie = np.random.random((c.Ni, c.Ne))

s.Wve = task.generate_input_weights()
s.Mve = np.where(s.Wve, True, False)

s.Wee = random_pos_sparse_conn((c.Ne, c.Ne), c.lambda_w)
s.Mee = np.where(s.Wee, True, False)

s.Te = c.Te_min + np.random.random((c.batch_size, c.Ne))*(c.Te_max - c.Te_min)
s.Ti = c.Ti_min + np.random.random((c.batch_size, c.Ni))*(c.Ti_max - c.Ti_min)

synaptic_normalization(s.Wee)
synaptic_normalization(s.Wei)
synaptic_normalization(s.Wie)
synaptic_normalization(s.Wve)

silence_duration = 20

def run(pattern_id, c, s):
    x = np.zeros((c.batch_size, c.Ne))
    y = np.zeros((c.batch_size, c.Ni))
    u = np.zeros((c.batch_size, task.N_a))
    silence = np.zeros((c.batch_size, task.N_a))

    pattern = np.expand_dims(task.create_pattern(pattern_id), 1)
    print(pattern)

    Te_h = np.zeros((c.n_steps, c.batch_size, c.Ne))
    x_h = np.zeros((c.n_steps, c.batch_size, c.Ne))
    y_h = np.zeros((c.n_steps, c.batch_size, c.Ni))
    Wee_h = np.zeros((c.n_steps, c.Ne, c.Ne))
    dWee_h = np.zeros((c.n_steps, c.Ne, c.Ne))
    dWee_m_h = np.zeros((c.n_steps,))
    Wev_h = np.zeros((c.n_steps, task.N_a, c.Ne))

    for step_id in range(c.n_steps):
        p_id = step_id % (pattern.shape[0] + silence_duration)

        if p_id < pattern.shape[0]:
            u_new = pattern[p_id]
            input = np.dot(u_new, s.Wve)
        else:
            u_new = silence
            input = 0.0

        R = np.dot(x, s.Wee) - np.dot(y, s.Wie)

        x_new = heavyside(R + c.input_gain * input - s.Te)
        y_new = heavyside(np.dot(x, s.Wei) - s.Ti)

        dWee = stdp(x, x_new)
        dWve = stdp(u, u_new, x, x_new)

        s.Te += c.eta_ip * (x - c.h_ip)

        s.Wee[s.Mee] += c.eta_stdp * dWee[s.Mee]
        s.Wve[s.Mve] += c.eta_stdp * dWve[s.Mve]

        bound_weights(s.Wee, s.Mee, c)
        bound_weights(s.Wve, s.Mve, c)

        synaptic_normalization(s.Wee)
        synaptic_normalization(s.Wve)

        x_h[step_id] = x.copy()
        y_h[step_id] = y.copy()
        Te_h[step_id] = s.Te.copy()
        Wee_h[step_id] = s.Wee.copy()
        dWee_h[step_id] = dWee.copy()
        dWee_m_h[step_id] = np.linalg.norm(dWee)
        Wev_h[step_id] = s.Wve.copy()

        x = x_new
        y = y_new
        u = u_new

    return x_h, y_h, Te_h, Wee_h


x_h, y_h, Te_h, Wee_h = run(0, c, s)

x0_h, y0_h, _, _ = run(1, c.copy(eta_stdp=0.0, n_steps=5000), s)

x1_h, y1_h, _, _ = run(2, c.copy(eta_stdp=0.0, n_steps=5000), s)


shl(np.mean(x_h[-1000:], axis=0), np.mean(x0_h, axis=0), np.mean(x1_h, axis=0))

shm(x_h[-500:], x0_h[-500:], x1_h[-500:])

shm(y_h[-300:], y0_h[-300:], y1_h[-300:])
