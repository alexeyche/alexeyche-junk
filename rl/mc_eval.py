
import sys
import os
import gym
RF_REPO = "{}/distr/reinforcement-learning".format(os.environ["HOME"])
if RF_REPO not in sys.path:
    sys.path.append(RF_REPO)

from lib.envs.blackjack import BlackjackEnv
from collections import defaultdict
import numpy as np
from gym import spaces
from lib import plotting

env = BlackjackEnv()

def reverse_array(a):
    assert len(a.shape) == 1
    return np.fliplr([a])[0]


def sample_policy(observation):
    """
    A policy that sticks if the player score is >= 20 and hits otherwise.
    """
    score, dealer_score, usable_ace = observation
    return 0 if score >= 20 else 1


policy = sample_policy
discount_factor = 1.0

returns_sum = defaultdict(float)
returns_count = defaultdict(float)
V = defaultdict(float)
num_episodes = 10000
t_steps = 100

R = np.zeros((num_episodes, 1))

for i_episode in range(1, num_episodes + 1):
    episode = []
    state = env.reset()
    rewards = 0.0
    for t in range(t_steps):
        action = policy(state)
        next_state, reward, done, _ = env.step(action)
        episode.append((state, action, reward))

        if done:
            break
        state = next_state

    states_in_episode = set([s for s, _, _ in episode])
    for s in states_in_episode:
        first_occurence_idx = next(i for i, x in enumerate(episode) if x[0] == state)
        G = sum([
            r * (discount_factor ** i)
            for i, (_, _, r) in enumerate(episode[first_occurence_idx:])
        ])
        returns_sum[state] += G
        returns_count[state] += 1.0
        V[state] = returns_sum[state] / returns_count[state]


Va = np.asarray([(k, v) for k, v in V.items()])
ids = reverse_array(np.argsort(Va[:, 1]))

for k, v in Va[ids][:10]:
    print(k, v)



