

def binary_search(v, arr, lb=None, rb=None):
	if lb is None:
		lb = 0
	if rb is None:
		rb = len(arr)

	mp = (rb + lb)/2
	print lb, mp, rb
	if lb == mp:
		if rb >= len(arr):
			return lb
		d_rb = arr[rb]-v
		d_lb = v-arr[lb]
		return rb if d_rb < d_lb else lb
	if arr[mp] < v:
		return binary_search(v, arr, mp, rb)
	elif arr[mp] > v:
		return binary_search(v, arr, lb, mp)
	else:
		return mp




import numpy as np


def test_bs():
	for i in xrange(1000):
		arr_size = np.random.randint(1, 1000)
		arr = range(arr_size)
		v = np.random.randint(0, arr_size)

		ans = binary_search(v, arr)

		assert ans == arr.index(v), "Test failed: {} {}".format(arr, v)
		print "Done {} with array size {}".format(i, arr_size)


print binary_search(1.6, [1,2,3,4], 0, 4)