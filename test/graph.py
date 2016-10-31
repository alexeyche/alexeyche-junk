
from collections import deque

class Node(object):
	def __init__(self, val, idx):
		self.val = val
		self.idx = idx
		self.connected_with = []

	def __str__(self):
		return "(\"{}\")".format(self.val, self.idx)
	def __repr__(self):
		return str(self)


class Graph(object):
	def __init__(self, nodes):
		self.nodes = [ Node(node_val, idx) for idx, node_val in enumerate(nodes) ]
		

	def connect(self, i, j, w):
		assert (i < len(self.nodes)) and (j < len(self.nodes)), "Out of nodes array"
		assert i != j, "No self connections"
		
		self.nodes[i].connected_with.append((self.nodes[j], w))
		# self.nodes[j].connected_with.append((self.nodes[i], w))




def dfs(v):
	discovered_set = set()
	stack = [v]

	while len(stack) > 0:
		v = stack.pop()
		if not v.idx in discovered_set:
			print "Discovered {}".format(v)
			discovered_set.add(v.idx)
			for edge, weight in reversed(v.connected_with):
				stack.append(edge)


def dfs_recc(v, discovered_set):
	if v.idx not in discovered_set:
		print "Discovered {}".format(v)
		discovered_set.add(v.idx)
		for edge, weight in v.connected_with:
			dfs_recc(edge, discovered_set)



def bfs(v):
	queue = deque([v])
	discovered_set= set([v.idx])
	
	while len(queue) > 0:
		v = queue.popleft()
		for edge, weight in v.connected_with:
			if not edge.idx in discovered_set:
				discovered_set.add(edge.idx)
				print "Discovered {}".format(edge)
				queue.append(edge)
			


G = Graph(["1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"])



G.connect(0, 1, 1.0)
G.connect(0, 2, 1.0)
G.connect(0, 3, 1.0)
G.connect(1, 4, 1.0)
G.connect(1, 5, 1.0)
G.connect(3, 6, 1.0)
G.connect(3, 7, 1.0)
G.connect(4, 8, 1.0)
G.connect(4, 9, 1.0)
G.connect(6, 10, 1.0)
G.connect(6, 11, 1.0)


for n in G.nodes:
	print "{} connected with:".format(n)
	for cn, w in n.connected_with:
		print "\t{}".format(cn)
s
print "DFS:"
dfs(G.nodes[0])
print
print "BFS:"
bfs(G.nodes[0])
print 
print "DFS reccurent: "
dfs_recc(G.nodes[0], set([]))