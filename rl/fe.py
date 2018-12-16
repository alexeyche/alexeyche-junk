
import numpy as np
from util import *
from os.path import join as pj
import pandas as pd
import os
from fe_env import *
from transform import *

import logging
import sys

from sklearn.model_selection import train_test_split

from sklearn.metrics import accuracy_score
from sklearn.metrics import roc_auc_score
from sklearn.metrics import f1_score

from sklearn.linear_model import SGDClassifier


log_level = logging.INFO
root = logging.getLogger()
root.setLevel(log_level)
handler = logging.StreamHandler(sys.stdout)
handler.setLevel(log_level)
handler.setFormatter(
    logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
)
root.addHandler(handler)

seed = None
np.random.seed(seed)

AML_WD = pj(os.environ["HOME"], "aml")
ds = pd.read_csv(
    pj(AML_WD, "datasets/pima_indians.csv"),
    names= ["f{}".format(id) for id in range(8)] + ["target"]
)

for c in ds.columns:
    if ds[c].dtype in set((np.dtype("int64"), np.dtype("i"))):
        ds[c] = ds[c].astype(np.float64)

x, y = ds.drop("target", axis=1), ds["target"]




def random_policy(nN, nA, epsilon):
    def policy_fn(observations):
        A = np.ones((len(observations), nA), dtype=float) / (nA * nN)
        return A, 0.0
    return policy_fn


def Q_calc(observations, W):
    return np.asarray([
        np.dot(W, observation)
        for observation in observations
    ])

def argmax_2d(a):
    return np.unravel_index(np.argmax(a.reshape(-1)), a.shape)

def make_epsilon_greedy_policy(W, epsilon, nA):
    def policy_fn(observations):
        Q = Q_calc(observations, W)
        A = np.ones((len(observations), nA), dtype=float) * epsilon / (len(observations) * nA)

        best_action = argmax_2d(Q)

        A[best_action] += (1.0 - epsilon)
        return A, Q
    return policy_fn




def evaluate(xm):
    x_train, x_test, y_train, y_test = train_test_split(
        xm, y,
        test_size = 0.25,
        random_state = 2,
    )

    cf = SGDClassifier(random_state=2, max_iter=1000, tol=1e-03)
    cf.fit(x_train, y_train)
    y_test_hat = cf.predict(x_test)
    return metric(y_test, y_test_hat)

def choice(probs):
    probs_flat = probs.reshape(-1)
    return np.unravel_index(
        np.random.choice(np.arange(len(probs_flat)), p=probs_flat),
        probs.shape
    )


metric = roc_auc_score

epsilon = 0.1
alpha = 0.005
sigma = 0.99
epochs = 1000

max_steps = 10

fe = FeatureEngineering(x, evaluate, max_steps=max_steps)
# W = np.ones((fe.num_of_transforms, fe.state_space_size))
W = 0.1 * (np.random.random((fe.num_of_transforms, fe.state_space_size)) - 0.5)


r_epoch_h = np.zeros((epochs, ))
for epoch in range(epochs):
    fe.reset()

    # policy = random_policy(fe.num_of_nodes, fe.num_of_transforms, epsilon)
    policy = make_epsilon_greedy_policy(W, epsilon, fe.num_of_transforms)

    r_h = np.zeros((max_steps, ))
    state = np.asarray([np.ones((fe.state_space_size,))])
    for iter_id in range(max_steps):
        probs, Q = policy(state)
        node_id, action_id = choice(probs)

        new_state, r, done, _ = fe.step(node_id, action_id)

        Q_new = Q_calc(new_state, W)
        Q_new_max = Q_new[argmax_2d(Q_new)]

        W[action_id] += alpha * (r + sigma * Q_new_max - Q[node_id, action_id]) * W[action_id]

        r_h[iter_id] = r
        state = new_state

    r_epoch_h[epoch] = np.mean(r_h)

    if epoch % 10 == 0:
        print("Epoch {}, R {}".format(epoch, r_epoch_h[epoch]))


# SUM are summing across strange dimensions


