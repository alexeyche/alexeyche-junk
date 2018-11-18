
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
from util import *
import gc

env = BlackjackEnv()

def reverse_array(a):
    assert len(a.shape) == 1
    return np.fliplr([a])[0]


Q = defaultdict(lambda: np.zeros(env.action_space.n))
returns_sum = defaultdict(float)
returns_count = defaultdict(float)

num_episodes = 500000
sample_freq = 10000
R0 = np.zeros((num_episodes // sample_freq,))
R1 = np.zeros((num_episodes // sample_freq,))

discount_factor = 1.0
t_steps = 100
epsilon = 0.1

def make_epsilon_greedy_policy(Q, epsilon, nA):
    def policy_fn(observation):
        A = np.ones(nA, dtype=float) * epsilon / nA
        best_action = np.argmax(Q[observation])
        A[best_action] += (1.0 - epsilon)
        return A
    return policy_fn


def random_policy(nA, epsilon):
    def policy_fn(observation):
        A = np.ones(nA, dtype=float) * epsilon / nA
        best_action = np.argmax(np.zeros(nA))
        A[best_action] += (1.0 - epsilon)
        return A
    return policy_fn


def evaluate_policy(policy, samples=10000):
    all_rewards = 0.0
    for _ in range(samples):
        rewards = 0.0
        state = env.reset()
        for t in range(t_steps):
            probs = policy(state)
            action = np.random.choice(np.arange(len(probs)), p=probs)

            next_state, reward, done, _ = env.step(action)
            rewards += reward
            if done:
                break
            state = next_state

        all_rewards += rewards / t_steps
    return all_rewards / samples



def run(num_episodes = 10000, epsilon = 0.0):

    for i_episode in range(1, num_episodes + 1):
        policy = make_epsilon_greedy_policy(Q, epsilon, env.action_space.n)

        episode = []
        state = env.reset()
        for t in range(t_steps):
            probs = policy(state)
            action = np.random.choice(np.arange(len(probs)), p=probs)

            next_state, reward, done, _ = env.step(action)
            episode.append((state, action, reward))
            if done:
                break
            state = next_state


        if i_episode % sample_freq == 0:
            r0 = evaluate_policy(random_policy(env.action_space.n, epsilon))
            r1 = evaluate_policy(policy)
            R0[(i_episode - 1) // sample_freq] = r0
            R1[(i_episode - 1) // sample_freq] = r1
            print(i_episode, r0, r1)

        for s, a in set([(s, a)for s, a, r in episode]):
            first_occ_id = next(
                i
                for i, x in enumerate(episode)
                if (x[0], x[1]) == (s, a)
            )
            G = sum([
                r * (discount_factor ** i)
                for i, (_, _, r) in enumerate(episode[first_occ_id:])
            ])

            returns_sum[(s, a)] += G
            returns_count[(s, a)] += 1.0
            Q[s][a] = returns_sum[(s, a)] / returns_count[(s, a)]


run(num_episodes, epsilon=0.1)

# r0 = evaluate_policy(random_policy(env.action_space.n, epsilon), samples=10000)
# r1 = evaluate_policy(make_epsilon_greedy_policy(Q, 0.0, env.action_space.n), samples=10000)
# print(r0, r1)


# shl(pd.Series(R).rolling(10000).mean())

# V = defaultdict(float)
# for state, actions in Q.items():
#     action_value = np.max(actions)
#     V[state] = action_value
# plotting.plot_value_function(V, title="Optimal Value Function")

# Va = np.asarray([(k, v) for k, v in V.items()])
# ids = reverse_array(np.argsort(Va[:, 1]))

# for k, v in Va[ids][:20]:
#     print(k, v)


