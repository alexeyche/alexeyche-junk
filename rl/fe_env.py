
import numpy as np
from collections import deque
from itertools import islice
import pandas as pd
import functools as ft
import tempfile
import logging
from collections import defaultdict
from transform import TIdentity

class Node(object):
    class Features(object):
        def __init__(self):
            self.reward = None
            self.feature_counts = None

    def __init__(self, id, transformation, parent, descendants=None):
        self.transformation = transformation
        self.descendants = list() if descendants is None else descendants
        self.id = id
        self.features = Node.Features()
        self.parent = parent

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "{}{} (R={})".format(
            self.transformation.__name__,
            self.id,
            (
                "{:.4f}".format(self.features.reward)
                if self.features.reward is not None else "NA"
            )
        )

    def __eq__(self, a):
        return self.id == a.id

    def __hash__(self):
        return hash(self.id)


log = logging.getLogger(__name__)

class FeatureEngineering(object):
    class Statistics(object):
        def __init__(self):
            self.reward_moments = {}

    def reset(self):
        self.steps_done = 0
        self.statistics = FeatureEngineering.Statistics()

        def identity(x):
            return x

        self.nodes = [
            Node(0, TIdentity, None)
        ]

    def __init__(self, dataset, evaluate, transformations, max_steps):
        self.transformations = transformations
        self.evaluate_callback = evaluate
        self.dataset = dataset
        self.max_steps = max_steps
        self.reset()


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
        n = Node(len(self.nodes), t, self.nodes[node_id])
        self.nodes.append(n)
        self.nodes[node_id].descendants.append(n)
        return n

    def apply(self, concat=True):
        global_ops = defaultdict(dict)

        def apply_ts(nodes_visited):
            D = self.dataset
            for n in nodes_visited:
                if n.transformation.is_semigroup:
                    global_ops[n.transformation][n] = D
                else:
                    D_new = n.transformation(D)
                    if D_new is None:
                        n.features.feature_counts = 0
                        break
                    else:
                        n.features.feature_counts = D_new.shape[1]
                        D = D_new
            return D

        res = self.traverse(apply_ts)

        for global_op, node_to_arg in global_ops.items():
            log.debug("Applying data for semigroup transform {} from {}".format(
                global_op,
                ",".join([str(n) for n in node_to_arg.keys()])
            ))

            D = global_op(*list(node_to_arg.values()))
            res.append(D)
            for n in node_to_arg.keys():
                n.features.feature_counts = D.shape[1]

        if concat:
            return np.concatenate(res, axis=1)
        else:
            return res

    def evaluate(self):
        result = self.apply()
        return self.evaluate_callback(result)


    def get_observations(self, n):
        assert n.features.feature_counts is not None, \
            "Node {} has empty feature counts".format(n)

        def walk_to_root(np, tused, depth):
            if np == self.root:
                return tused, depth
            elif np.transformation.name == n.transformation.name:
                tused += 1.0
            depth += 1.0
            return walk_to_root(np.parent, tused, depth)

        tused, depth = walk_to_root(n.parent, 0, 0)
        if n.parent == self.root:
            reward_gain = n.features.reward
        else:
            reward_gain = n.features.reward - n.parent.features.reward

        r0, r1 = (
            self.statistics.reward_moments[n.transformation.name]
        )

        return np.asarray((
            n.features.reward,
            r1 / r0,
            tused,
            reward_gain,
            depth,
            self.steps_done / float(self.max_steps),
            n.features.feature_counts / float(self.dataset.shape[1]),
            # is_t_feature_sel,
            # feature_type,
        ))

    def step(self, node_id, action):
        assert node_id < len(self.nodes)

        t = self.transformations[action]
        n = self.add_transformation(node_id, t)
        r = self.evaluate()
        n.features.reward = r
        done = False

        sum, count = (
            self.statistics.reward_moments.get(t.name, (0.0, 0.0))
        )
        self.statistics.reward_moments[t.name] = count + 1.0, sum + r

        obs = self.get_observations(n)
        self.steps_done += 1
        return obs, r, done, {}


    def to_graphviz(self):
        from graphviz import Digraph
        dot = Digraph(comment="FeatureEngineering Graph")
        for n in self.nodes:
            dot.node(str(n))

        def _walk(nv):
            edges = set()
            pair_it = zip(
                islice(nv, 0, len(nv) - 1), islice(nv, 1, len(nv))
            )
            for n0, n1 in pair_it:
                edges.add((str(n0), str(n1)))
            return edges

        res = self.traverse(_walk)
        uniq_edges = ft.reduce(lambda x, y: x | y, res)

        for n0, n1 in uniq_edges:
            dot.edge(n0, n1)

        return dot

    def plot(self):
        (
            self
                .to_graphviz()
                .render(
                    tempfile.mktemp(".png"),
                    view=True,
                    format="png"
                )
        )


