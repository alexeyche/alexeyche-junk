
from matplotlib import pyplot as plt
from matplotlib.pyplot import cm
import numpy as np

from common import Determ, BioLearning

np.random.seed(1)

lrate = 1e-01
inp_spikes = [25.0]

T = 1
lrule = BioLearning.STDP

sim_time = 100

epochs = 1
dt = 1.0
y_sp_t = 26.0

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


tau_syn = 10.0
tau_ref = 2.0
threshold = 0.5

act = Determ(threshold)

def alpha(x, tau):
	return np.exp(-x/tau)




def srm(t, W, inp_spikes, out_spikes):
	x_syn = 0
	for t_sp in inp_spikes[:]:
		t_sp += 1.0 # dendrite time 
		if t_sp > t:
			break
		s_sp = t - t_sp
		if s_sp < tau_syn*10:
			x_syn += alpha(s_sp, tau_syn)
		else:
			inp_spikes.pop(0)

	y = x_syn * W
	if len(out_spikes) > 0:
		y += -100.0 * alpha(t-out_spikes[-1], tau_ref)
	p_act = act(y)
	if p_act > np.random.random():
		out_spikes.append(t)
	return y, p_act

acc0, acc1 = [], []
spikes0, spikes1 = [], []

for t in xrange(sim_time):
	y0, p_act0 = srm(t, W0, inp_spikes, spikes0)
	y1, p_act1 = srm(t, W1, spikes0, spikes1)
	
	acc0.append(y0)
	acc1.append(y1)

