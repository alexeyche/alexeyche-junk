
import numpy as np
import random

class CountingTask(object):
    def __init__(self, words, N, N_used, avoid=False):
        self.words = words

        self.alphabet = np.unique(list("".join(words)))
        self.N_a = self.alphabet.shape[0]
        self.lookup = dict(zip(self.alphabet, range(self.N_a)))
        self.N = N
        self.N_used = N_used
        self.avoid = avoid

    def generate_input_weights(self):
        W = np.zeros((self.N_a, self.N))

        available = set(range(self.N))
        for a in range(self.N_a):
            temp = random.sample(available, self.N_used)
            W[a, temp] = 1
            if self.avoid:
                available = available.difference(temp)

        # The underscore has the special property that it doesn't
        # activate anything:
        if '_' in self.lookup:
            W[self.lookup['_'], :] = 0
        return W

    def create_pattern(self, i):
        w = self.words[i]
        pattern = np.zeros((len(w), self.N_a))
        for li, c in enumerate(w):
            pattern[li, self.lookup[c]] = 1.0
        return pattern


