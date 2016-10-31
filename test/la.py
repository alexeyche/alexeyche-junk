from itertools import chain
import numpy as np


def transpose(m):
	for i in xrange(m.rows):
		for j in xrange(i):
			if i != j:
				# print i, j, ": ", m[i,j], "->", m[j,i]
				tmp = m[i,j]
				m[i,j] = m[j,i]
				m[j,i] = tmp
				# return


class Matrix(object):
	def __init__(self, vals, cols, rows):
		self.vals = vals
		self.cols = cols
		self.rows = rows
		self.transposed = False
		self.rotated = False

	def __str__(self):
		s = ""
		for i in xrange(self.rows):
			for j in xrange(self.cols):
				 s += "{}, ".format(self[i,j])
			s += "\n"
		return s

	def transpose(self):
		tmp = self.cols
		self.cols = self.rows
		self.rows = tmp
		self.transposed = not self.transposed

	def __construct_key(self, k):
		if self.transposed:
			acc = k[0]
			if self.rotated:
				acc = self.rows - k[0] - 1
			return k[1]*self.rows + acc
		else:
			acc = k[1]
			if self.rotated:
				acc = self.cols - k[1] - 1
			return k[0]*self.cols + acc

	def __getitem__(self, k):
		return self.vals[self.__construct_key(k)]
	
	def __setitem__(self, k, v):
		self.vals[self.__construct_key(k)] = v 


# vals = np.random.random(10)
vals = range(0, 10)
m = Matrix(
	vals,
	2,
	5
)
# print m
# transpose(m)
# print m

print m
m.transpose()
print m
m.rotated = True
print m 