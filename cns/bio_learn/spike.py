from matplotlib import pyplot as plt
from matplotlib.pyplot import cm
import numpy as np
from functools import partial

from common import Determ, Learning
import scipy.sparse as sp

def get_idx_of_refractory(ti, spikes, tau_ref):
	sp_idx = spikes[max(0, ti-int(tau_ref/dt)):ti].rows
   	if len(sp_idx) > 0:
   		return [ int(v) for v in np.hstack(sp_idx) ]
   	return np.asarray([], dtype=np.uint32)
    

np.random.seed(10)

lrate = 1.0
epochs = 1000

in_size = 10
hidden_size0 = 10
out_size = 1


W0 = np.random.rand(in_size, hidden_size0)
W1 = np.random.rand(hidden_size0, out_size)
B0 = np.random.rand(out_size, hidden_size0)

W0 *= 4.0
W1 *= 4.0

# W2 = -0.1 + 0.2*np.random.rand(hidden_size1, out_size)

tau_syn = 10.0
tau_ref = 2.0
tau_learn = 5.0
tau_mem = 2.0
threshold = 1.0

act = Determ(threshold)

lrule = Learning.BP



F = -0.1 + 0.2*np.random.rand(in_size, out_size)

T = 100
dt = 1.0
Tsize = int(T/dt)
#x_s = np.random.randn(Tsize, in_size)
x_s = np.zeros((Tsize, in_size))
# tt = 0
# while tt < T:
# for ni in xrange(in_size):
#     x_s[tt, ni] = in_size
    # tt += 2.0
x_s[0,1] = 1.0
x_s[10,0] = 1.0
x_s[20,2] = 1.0
x_s[21,0] = 1.0
x_s[30,0] = 1.0
x_s[35,1] = 1.0
x_s[1,3] = 1.0
x_s[15,4] = 1.0
x_s[25,5] = 1.0
x_s[30,3] = 1.0
x_s[37,4] = 1.0
x_s[50,5] = 1.0
x_s[5,6] = 1.0
x_s[12,7] = 1.0
x_s[18,8] = 1.0
x_s[23,9] = 1.0
x_s[30,7] = 1.0
x_s[40,6] = 1.0

y_t_s = np.zeros((Tsize, out_size)) #Determ(0.75)(np.dot(x_s, F))
y_t_s[0,0] = 1.0
y_t_s[25,0] = 1.0
y_t_s[50,0] = 1.0

stats = []
error_acc = []
for epoch in xrange(epochs):
    x_mean = np.zeros(in_size)
    u0 = np.zeros(hidden_size0)
    s0 = np.zeros(hidden_size0)
    u0m = np.zeros(hidden_size0)

    u1 = np.zeros(out_size)

    ym = np.zeros(out_size)
    ym_t = np.zeros(out_size)


    u0_stat = np.zeros((Tsize, hidden_size0))
    u0m_stat = np.zeros((Tsize, hidden_size0))
    u1_stat = np.zeros((Tsize, out_size))
    e_stat = np.zeros((Tsize, out_size))
    s0_stat = np.zeros((Tsize, hidden_size0))
    
    spikes0 = sp.lil_matrix((Tsize, hidden_size0), dtype=np.float32)
    spikes1 = sp.lil_matrix((Tsize, out_size), dtype=np.float32)

    dW0_stat, dW1_stat = np.zeros((Tsize, W0.shape[0], W0.shape[1])), np.zeros((Tsize, W1.shape[0], W1.shape[1]))
    dW0, dW1 = np.zeros(W0.shape), np.zeros(W1.shape)

    int_error = 0.0
    for ti, t in enumerate(np.linspace(0, T, Tsize)):
        target_fired = y_t_s[ti,:]
        x = x_s[ti, :]
        
        x_mean += dt * ( - x_mean/tau_syn + x )

        u0 += dt * (- u0 + np.dot(x_mean, W0))/tau_mem
    	u0[get_idx_of_refractory(ti, spikes0, tau_ref)] = -5.0
        a0 = act(u0)
        s0 += dt * (- s0/tau_syn + a0)
        spikes0[ti, np.where(a0 == 1.0)] = 1.0
        u0m += dt * (- u0m/tau_learn + a0)

        s0_stat[ti, :] = s0

        u1 += dt * (- u1 + np.dot(s0, W1))/tau_mem
    	u1[get_idx_of_refractory(ti, spikes1, tau_ref)] = -5.0
        a1 = act(u1)
        spikes1[ti, np.where(a1 == 1.0)] = 1.0

        ym += dt * (- ym/tau_learn + a1)
        ym_t += dt * (- ym_t/tau_learn + target_fired)

        u0_stat[ti, :] = u0
        u0m_stat[ti, :] = u0m
        u1_stat[ti, :] = u1
        
        e = ym - ym_t
        e_stat[ti, :] = e

        error = 0.5 * np.inner(e, e)

        int_error += error/Tsize

        if lrule == Learning.BP or lrule == Learning.FA:
            if lrule == Learning.BP:
                du0 = np.dot(W1, e)  #* act.deriv(u0)
            else:
                du0 = np.dot(e, B0) #* act.deriv(u0)
            
            dW0_cur = - np.outer(x_mean, du0)/Tsize
            dW1_cur = - np.outer(s0, e)/Tsize
            
            dW0 += dW0_cur
            dW1 += dW1_cur

            dW0_stat[ti, :, :] = dW0_cur
            dW1_stat[ti, :, :] = dW1_cur

    W0 += lrate * dW0
    W1 += lrate * dW1

    error_acc.append(int_error)
    print "Epoch {}, error {}".format(epoch, int_error) 


# plt.plot(stats[0], "r")
# plt.show()

# plt.subplot(2,1,1)
# plt.imshow(y_t_s.T)
# plt.subplot(2,1,2)
# plt.imshow(spikes1.todense().T)
# plt.show()

    hni = 0
    plt.figure(1, figsize=(20, 10), dpi=10)
    plt.subplot(8,1,1)
    plt.title("Target")
    plt.plot(y_t_s[:,0])
    plt.subplot(8,1,2)
    plt.title("Actual")
    plt.plot(spikes1.todense()[:,0], linewidth=5.0)
    plt.subplot(8,1,3)
    plt.title("Hidden act")
    plt.imshow(spikes0.todense().T)
    plt.subplot(8,1,4)
    plt.title("Error")
    plt.plot(e_stat[:,0])
    plt.subplot(8,1,5)
    plt.title("Synapses")
    plt.plot(x_s[:,0])
    plt.plot(x_s[:,1])
    plt.plot(x_s[:,2])
    plt.plot(x_s[:,3])
    plt.plot(x_s[:,4])
    plt.plot(x_s[:,5])
    plt.plot(x_s[:,6])
    plt.plot(x_s[:,7])
    plt.plot(x_s[:,8])
    plt.plot(x_s[:,9])
    plt.subplot(8,1,6)
    hni = 0
    plt.title("Derivative {}".format(hni))
    plt.plot(dW0_stat[:,0,hni])
    plt.plot(dW0_stat[:,1,hni])
    plt.plot(dW0_stat[:,2,hni])
    plt.plot(dW0_stat[:,3,hni])
    plt.plot(dW0_stat[:,4,hni])
    plt.plot(dW0_stat[:,5,hni])
    plt.plot(dW0_stat[:,6,hni])
    plt.plot(dW0_stat[:,7,hni])
    plt.plot(dW0_stat[:,8,hni])
    plt.plot(dW0_stat[:,9,hni])
    # plt.subplot(8,1,6)
    # hni = 1
    # plt.title("Derivative {}".format(hni))
    # plt.plot(dW0_stat[:,0,hni])
    # plt.plot(dW0_stat[:,1,hni])
    # plt.plot(dW0_stat[:,2,hni])
    # plt.plot(dW0_stat[:,3,hni])
    # plt.plot(dW0_stat[:,4,hni])
    # plt.plot(dW0_stat[:,5,hni])
    # plt.plot(dW0_stat[:,6,hni])
    # plt.plot(dW0_stat[:,7,hni])
    # plt.plot(dW0_stat[:,8,hni])
    # plt.plot(dW0_stat[:,9,hni])
    # plt.subplot(8,1,7)
    # hni = 2
    # plt.title("Derivative {}".format(hni))
    # plt.plot(dW0_stat[:,0,hni])
    # plt.plot(dW0_stat[:,1,hni])
    # plt.plot(dW0_stat[:,2,hni])
    # plt.plot(dW0_stat[:,3,hni])
    # plt.plot(dW0_stat[:,4,hni])
    # plt.plot(dW0_stat[:,5,hni])
    # plt.plot(dW0_stat[:,6,hni])
    # plt.plot(dW0_stat[:,7,hni])
    # plt.plot(dW0_stat[:,8,hni])
    # plt.plot(dW0_stat[:,9,hni])
    # plt.subplot(8,1,8)
    # hni = 3
    # plt.title("Derivative {}".format(hni))
    # plt.plot(dW0_stat[:,0,hni])
    # plt.plot(dW0_stat[:,1,hni])
    # plt.plot(dW0_stat[:,2,hni])
    # plt.plot(dW0_stat[:,3,hni])
    # plt.plot(dW0_stat[:,4,hni])
    # plt.plot(dW0_stat[:,5,hni])
    # plt.plot(dW0_stat[:,6,hni])
    # plt.plot(dW0_stat[:,7,hni])
    # plt.plot(dW0_stat[:,8,hni])
    # plt.plot(dW0_stat[:,9,hni])
    # plt.subplot(8,1,9)
    # hni = 4
    # plt.title("Derivative {}".format(hni))
    # plt.plot(dW0_stat[:,0,hni])
    # plt.plot(dW0_stat[:,1,hni])
    # plt.plot(dW0_stat[:,2,hni])
    # plt.plot(dW0_stat[:,3,hni])
    # plt.plot(dW0_stat[:,4,hni])
    # plt.plot(dW0_stat[:,5,hni])
    # plt.plot(dW0_stat[:,6,hni])
    # plt.plot(dW0_stat[:,7,hni])
    # plt.plot(dW0_stat[:,8,hni])
    # plt.plot(dW0_stat[:,9,hni])
    plt.subplot(8,1,7)
    plt.title("Hidden")
    plt.plot(s0_stat[:,0], linewidth=5.0)
    plt.plot(s0_stat[:,1])
    plt.plot(s0_stat[:,2])
    plt.plot(s0_stat[:,3])
    plt.plot(s0_stat[:,4])
    plt.plot(s0_stat[:,5])
    plt.plot(s0_stat[:,6])
    plt.plot(s0_stat[:,7])
    plt.plot(s0_stat[:,8])
    plt.plot(s0_stat[:,9])
    plt.subplot(8,1,8)
    plt.title("Derivative 1")
    plt.plot(dW1_stat[:,0,0], linewidth=5.0)
    plt.plot(dW1_stat[:,1,0])
    plt.plot(dW1_stat[:,2,0])
    plt.plot(dW1_stat[:,3,0])
    plt.plot(dW1_stat[:,4,0])
    plt.plot(dW1_stat[:,5,0])
    plt.plot(dW1_stat[:,6,0])
    plt.plot(dW1_stat[:,7,0])
    plt.plot(dW1_stat[:,8,0])
    plt.plot(dW1_stat[:,9,0])

    plt.savefig("/home/alexeyche/prog/tmp/{}_spike_plot.png".format(epoch), dpi=100)
    plt.clf()

stats.append(error_acc)

