#!/usr/bin/env python

import numpy as np
import time


def split(arr):
	spl_point = len(arr)/2
	return arr[:spl_point], arr[spl_point:]

def merge_seqs(larr, rarr):
	ans = []
	
	l_it, r_it = 0, 0
	while l_it < len(larr) and r_it < len(rarr):
		if larr[l_it] < rarr[r_it]:
			ans.append(larr[l_it])
			l_it += 1
		else:
			ans.append(rarr[r_it])
			r_it += 1

	ans += larr[l_it:]
	ans += rarr[r_it:]
	return ans

def merge_sort(arr):
	assert len(arr) > 0, "Zero array in input"
	if len(arr) == 2:
		return arr if arr[0] < arr[1] else [arr[1], arr[0]]
	elif len(arr) == 1:
		return arr
	else:
		larr, rarr = split(arr)
		larr_s = merge_sort(larr)
		rarr_s = merge_sort(rarr)
		return merge_seqs(larr_s, rarr_s)


def naive_bubble_sort(arr):
	for i in xrange(len(arr)):
		for j in xrange(len(arr)):
			if arr[i] < arr[j]:
				arr[i], arr[j] = arr[j], arr[i] # swap
	return arr


def bubble_sort(arr):
	for i in xrange(len(arr)):
		for j in xrange(len(arr)-1 - i):
			j_l, j_r = j, j+1
			if arr[j_l] > arr[j_r]:
			 	arr[j_l], arr[j_r] = arr[j_r], arr[j_l] # swap

	return arr



def insertion_sort(arr):
	for i in xrange(1, len(arr)):
		x = arr[i]
		j = i - 1
		while j >= 0 and arr[j] > x:
			arr[j+1] = arr[j]
			j -= 1
		arr[j+1] = x
	return arr



def test_sort(fun):
	start = time.time()
	for tit in xrange(1000):
		arr = np.random.randint(0, 1000, np.random.randint(1, 1000)).tolist()
		assert fun(arr) == sorted(arr), "Failed: {}".format(arr)
	print "Done {} in {} seconds".format(fun.__name__, time.time() - start)




# test_sort(merge_sort)
# test_sort(bubble_sort)
# test_sort(naive_bubble_sort)
# test_sort(insertion_sort)




# test_sort(merge_sort)


