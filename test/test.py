


class Node(object):
	def __init__(self, value, *childs):
		self.value = value
		self.childs = list(childs)
		
		for c in self.childs:
			c.parent = self
		
		self.parent = None
		self.depth = None
	def __repr__(self):
		return "Node({})".format(self.value)


n666 = Node(666)
n66, n77, n88 = Node(66), Node(77), Node(88)

n6, n7, n8 = Node(6, n66, n666), Node(7, n77), Node(8, n88)

root = Node(5, n6, n7, n8)


def dfs(node, depth=0):
	node.depth = depth
	print node.value, "depth {}".format(node.depth)
	for c in node.childs:
		dfs(c, depth + 1)




def lca(u, v):
	h1 = u.depth
	h2 = v.depth

	while h1 != h2:
		if h1 > h2:
			u = u.parent
			h1 -= 1
		else:
			v = v.parent
			h2 -= 1

	while u.value != v.value:
		u = u.parent
		v = v.parent

		assert not v is None and not u is None, "Can't find common ancestor"

	return u





def merge_seqs(s1, s2):
	ans = []

	si, sj = 0, 0
	while si < len(s1) and sj < len(s2):
		if s1[si] < s2[sj]:
			ans.append(s1[si])
			si += 1
		else:
			ans.append(s2[sj])
			sj += 1


	while si < len(s1):
		ans.append(s1[si])
		si +=1
	
	while sj < len(s2):
		ans.append(s2[sj])
		sj +=1
	return ans


def split(arr):
	spl_point = len(arr)/2
	return arr[:spl_point], arr[spl_point:]


def merge_sort(seq):
	if len(seq) == 1:
		return seq

	left_seq, right_seq = split(seq)
	
	left_seq = merge_sort(left_seq)	
	right_seq = merge_sort(right_seq)

	return merge_seqs(left_seq, right_seq)

# import random 
# for i in xrange(100):
# 	seq = [ random.randint(0, 1000) for _ in xrange(99) ]
# 	res = merge_sort(seq)
# 	assert res == sorted(seq), "Sorting is not working: {}".format(res)


# 	print "Good: {}".format(res)



def check_cyclic(node, visited, parent):
	visited[node.value] = True
	
	print "Visiting", node.value, visited, parent.value if parent else "None"
	for n in node.childs:
		print "Considering {}".format(n.value)
		if not visited.get(n.value, False):
			if check_cyclic(n, visited, node):
				return True
		else:
			return True			
	return False


def check_bipartite(node, visited, depth, cycle_level):
	visited[node.value] = set([depth % 2])
	
	print "Visiting", node.value, visited, cycle_level, depth
	for n in node.childs:
		print "Considering {}".format(n.value)
		if visited.get(n.value) is None:
			check_bipartite(n, visited, depth + 1, cycle_level)
		else:
			visited[n.value].add(depth % 2)
			# k = "{}->{}".format(node.value, n.value)
			# cycle_level[k] = depth + 1
	# return cycle_level

from collections import deque

def bfs(v):
	queue = deque([v])
	discovered_set= set([v.value])
	colors = {}
	colors[v.value] = 0

	while len(queue) > 0:
		v = queue.popleft()

		for edge in v.childs:
			if not edge.value in discovered_set:
				discovered_set.add(edge.value)
				colors[edge.value] = 1-colors[v.value]
				print "Discovered {}".format(edge)
				queue.append(edge)
			elif colors[edge.value] == colors[v.value]:
				print "Not bipartite for connection {}->{}, colors: {} == {}".format(edge.value, v.value, colors[edge.value], colors[v.value])

	return colors

n66, n77, n88 = Node(66), Node(77), Node(88)

n6, n7, n8 = Node(6), Node(7), Node(8)

def dep(l, r):
	l.childs.append(r)


dep(n6, n66)
dep(n7, n88)
dep(n8, n66)
dep(n6, n77)

dep(n66, n7)
dep(n77, n8)
dep(n88, n8)
dep(n88, n6)



# n6.childs.append(root)
# n666.childs.append(root)

# cycle_level = {}
# visited = {}
# check_bipartite(n6, visited, 0, cycle_level)	

# print visited

print bfs(n6)


# print check_cyclic(n6, {}, None)
