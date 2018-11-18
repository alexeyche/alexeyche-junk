
import numpy as np
from util import *
from os.path import join as pj
import pandas as pd
import os
from sklearn.model_selection import train_test_split

from sklearn.metrics import accuracy_score
from sklearn.metrics import roc_auc_score
from sklearn.metrics import f1_score

from sklearn.linear_model import SGDClassifier

seed = 0

metrics = (
    accuracy_score,
    roc_auc_score,
    f1_score,
)


AML_WD = pj(os.environ["HOME"], "aml")
ds = pd.read_csv(
    pj(AML_WD, "datasets/pima_indians.csv"),
    names= ["f{}".format(id) for id in range(8)] + ["target"]
)


train_ds, test_ds = train_test_split(
    ds,
    test_size = 0.25,
    random_state = seed,
)

train_x, train_y = train_ds.drop("target", axis=1), train_ds["target"]
test_x, test_y = test_ds.drop("target", axis=1), test_ds["target"]


transforms = (
    np.sin,
    np.log,
    np.exp,
)


def random_policy(nA, epsilon):
    def policy_fn(observation):
        A = np.ones(nA, dtype=float) * epsilon / nA
        best_action = np.argmax(np.zeros(nA))
        A[best_action] += (1.0 - epsilon)
        return A
    return policy_fn


epsilon = 0.1

policy = random_policy(len(transforms), epsilon)


d0 = train_x.copy()

probs = policy(None)
action = np.random.choice(np.arange(len(probs)), p=probs)


t = transforms[action]



cf = SGDClassifier(random_state=seed)
cf.fit(train_x, train_y)
test_y_hat = cf.predict(test_x)

for m in metrics:
    print(m.__name__, m(test_y, test_y_hat))


