
import numpy as np
from collections import deque
import pandas as pd

class Node(object):
    def __init__(self, id, transformation, descendants=None):
        self.transformation = transformation
        self.descendants = list() if descendants is None else descendants
        self.id = id

    def __repr__(self):
        return "{}:{}".format(self.id, self.transformation.__name__)



class FeatureEngineering(object):
    def __init__(self, dataset, evaluate, transformations):
        self.transformations = transformations
        self.evaluate_callback = evaluate
        self.dataset = dataset

        def identity(x):
            return x

        self.nodes = [
            Node(0, identity)
        ]

    @property
    def num_of_nodes(self):
        return len(self.nodes)

    @property
    def num_of_transforms(self):
        return len(self.transformations)

    def traverse(self, cb):
        nodes_visited = deque()
        result = []

        def visit(n):
            nodes_visited.append(n)

            if len(n.descendants) == 0:
                r = cb(nodes_visited)
                result.append(r)
            else:
                for dn in n.descendants:
                    visit(dn)

            nodes_visited.pop()

        visit(self.root)
        return result

    def __repr__(self):
        return "\n".join(
            [
                "{}: {}".format(id, l)
                for id, l in enumerate(self.traverse(
                    lambda ts: " -> ".join([str(t) for t in ts])
                ))
            ]
        )

    @property
    def root(self):
        return self.nodes[0]


    def add_transformation(self, node_id, t):
        n = Node(len(self.nodes), t)
        self.nodes.append(n)
        self.nodes[node_id].descendants.append(n)


    def apply(self, concat=True):
        def apply_ts(nodes_visited):
            D = self.dataset
            for n in nodes_visited:
                D = n.transformation(D)
            return D

        res = self.traverse(apply_ts)
        if concat:
            return np.concatenate(res, axis=1)
        else:
            return res

    def evaluate(self):
        result = self.apply()
        return self.evaluate_callback(result)


    def get_observations(self):
        return 0.0


    def step(self, node_id, action):
        assert node_id < len(self.nodes)

        t = self.transformations[action]
        self.add_transformation(node_id, t)

        r = self.evaluate()
        done = False

        return self.get_observations(), r, done, {}



class Transform(object):
    def __init__(self, filter, callback):
        self.filter = filter
        self.callback = callback

    def __call__(self, x):
        return self.callback(self.filter(x))

    @property
    def __name__(self):
        return self.callback.__name__


class Filters(object):
    @staticmethod
    def all():
        return lambda x: x

    @staticmethod
    def no_greater_than(upper):
        def cb(x):
            d = x.describe()
            return x[d.columns[d.loc["max"].lt(upper).values]]
        return cb

    @staticmethod
    def positive():
        def cb(x):
            d = x.describe()
            return x[d.columns[d.loc["min"].gt(0.0).values]]
        return cb


TSin = Transform(Filters.all(), np.sin)
TExp = Transform(Filters.no_greater_than(10.0), np.exp)
TSqrt = Transform(Filters.positive(), np.sqrt)
TLog = Transform(Filters.positive(), np.log)
TTanh = Transform(Filters.all(), np.tanh)





