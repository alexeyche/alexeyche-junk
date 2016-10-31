
from collections import deque


def topological(graph):
    GRAY, BLACK = 0, 1

    order, enter, state = deque(), set(graph), {}

    def dfs(n):
        state[n] = GRAY
        for k in graph.get(n, ()):
            sk = state.get(k, None)
            if sk == GRAY: raise ValueError("cycle")
            if sk == BLACK: continue
            enter.discard(k)
            dfs(k)
        order.appendleft(n)
        state[n] = BLACK

    while enter: dfs(enter.pop())
    return order


class Graph(object):
    def __init__(self):
        self.node_edges = {}
        self.root_nodes = []
        self.order = deque()

    def registry_root(self, node):
        self.root_nodes.append(node)

    def resfresh(self):
        self.node_edges = {}

        def traverse(n):
            self.node_edges[n] = n.vertices
            for d in n.vertices:
                traverse(d)

        for n in self.root_nodes:
            traverse(n)

        self.order = topological(self)
        for n_idx, n in enumerate(self.order):
            n.set_idx(n_idx)


    def get(self, n, default=()):
        return self.node_edges.get(n, default)

    def __iter__(self):
        return iter(self.node_edges.keys())

    def get_order(self):
        return self.order

graph = Graph()


