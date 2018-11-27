
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

root = logging.getLogger()
root.setLevel(logging.DEBUG)
handler = logging.StreamHandler(sys.stdout)
handler.setLevel(logging.DEBUG)
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

x, y = ds.drop("target", axis=1), ds["target"]

transforms = (
    TLog,
    TSin,
    TExp,
    TSqrt,
    TTanh,
    TSum
)

metric = roc_auc_score

epsilon = 0.1

def random_policy(nN, nA, epsilon):
    def policy_fn(observation):
        A = np.ones((nN, nA), dtype=float) / (nA * nN)
        return A
    return policy_fn



def evaluate(xm):
    x_train, x_test, y_train, y_test = train_test_split(
        xm, y,
        test_size = 0.25,
        random_state = seed,
    )

    cf = SGDClassifier(random_state=seed, max_iter=1000, tol=1e-03)
    cf.fit(x_train, y_train)
    y_test_hat = cf.predict(x_test)
    return metric(y_test, y_test_hat)

def choice(probs):
    probs_flat = probs.reshape(-1)
    return np.unravel_index(
        np.random.choice(np.arange(len(probs_flat)), p=probs_flat),
        probs.shape
    )


fe = FeatureEngineering(x, evaluate, transforms, max_steps=10)

max_iter = 10

r_h = np.zeros((max_iter, ))
obs = []
for iter_id in range(max_iter):
    policy = random_policy(fe.num_of_nodes, fe.num_of_transforms, epsilon)
    probs = policy(None)
    node_id, action_id = choice(probs)

    o, r, done, _ = fe.step(node_id, action_id)
    r_h[iter_id] = r
    obs.append(o)

obs = np.asarray(obs)
fe.plot()

# num_episodes = 100

# for i_episode in range(num_episodes):




