

import numpy as np
import pandas as pd
from util import *


def run_bandit(stat, k=10, sigma=0.1, max_iters=1000, optim_q=False, ucb=False, c=2.0):
    q_star_m = np.random.randn(k)

    def pull_arm(id):
        return q_star_m[id] + np.random.randn()

    A_opt = np.argmax(q_star_m)


    Q = 5.0 * np.ones((k,)) if optim_q else np.zeros((k,))
    N = np.zeros((k,))

    for t in range(max_iters):
        to_explore = np.random.random() < sigma
        if to_explore:
            A = np.random.randint(0, k)
        else:
            if ucb:
                addition = 100.0 * np.ones((k,))
                N_gt_one = N[np.where(N)]
                addition[np.where(N)] = c * np.sqrt(np.log(t + 1) / N_gt_one)
                A = np.argmax(Q + addition)
            else:
                A = np.argmax(Q)

        R = pull_arm(A)

        N[A] = N[A] + 1
        Q[A] = Q[A] + (R - Q[A]) / N[A]

        stat[t, 0] = R
        stat[t, 1] = np.linalg.norm(Q - q_star_m)
        stat[t, 2] = A == A_opt


sample_run = 500
max_iters = 2000


versions = [
    ("Optim init", {"sigma": 0.1, "optim_q": True}),
    ("sigma 0.1", {"sigma": 0.1}),
    ("UCB", {"ucb": True, "sigma": 0.1}),
    ("UCB greedy", {"ucb": True, "sigma": 0.0}),
    ("sigma 0.01", {"sigma": 0.01}),
    ("sigma 0.0", {"sigma": 0.0}),
]

stat = np.zeros((len(versions), sample_run, max_iters, 3))

for v_id, (name, args) in enumerate(versions):
    print("Running ", name)
    for s_id in range(sample_run):
        a = args.copy()
        a["max_iters"] = max_iters
        run_bandit(stat[v_id, s_id], **a)


stat = np.mean(stat, axis=1)


shl(
    *[
        pd.Series(stat[v_id, :, 0]).rolling(100).mean()
        for v_id in range(len(versions))
    ],
    labels=[name for name, _ in versions],
)

