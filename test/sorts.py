#!/usr/bin/env python

import numpy as np
import time
import random

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


def swap(arr, i, j):
	tmp = arr[j]
	arr[j] = arr[i]
	arr[i] = tmp

def qpartition(arr, lo, hi):
	pivot = arr[hi-1]
	# print "lo {} hi {}, pivot {}".format(lo, hi, pivot)
	# print "Beofore {}".format(arr[lo:hi])
	i = lo
	for j in xrange(lo, hi-1):
		# print "{}, {}: {} <= {} ? ".format(i, j, arr[j], pivot) 
		if arr[j] <= pivot:
			if i != j:
				swap(arr, i, j)
			i += 1		
	swap(arr, i, hi-1)
	# print "After {}".format(arr[lo:hi])
	return i

def qsort(arr, lo=None, hi=None):
	if lo is None:
		lo = 0
	if hi is None:
		hi = len(arr)
	if lo < hi:
		p = qpartition(arr, lo, hi)
		qsort(arr, lo, p)
		qsort(arr, p+1, hi)
	return arr


def test_sort(fun):
	start = time.time()
	for tit in xrange(1000):
		arr = np.random.randint(0, 1000, np.random.randint(1, 1000)).tolist()
		assert fun(arr) == sorted(arr), "Failed: {}".format(arr)
	print "Done {} in {} seconds".format(fun.__name__, time.time() - start)



def randomized_partition(A, p, r):
	if len(A) == 1:
		return 0
	pivot_idx = random.randint(p, r-1)
	swap(A, r-1, pivot_idx)
	return qpartition(A, p, r)

def randomized_select(A, p, r, i):
	if p == r:
		return A[p]
	q = randomized_partition(A, p, r)
	# print A
	# print q, p, r, i # 1, 0, 7, 1, 1-0
	k = q - p + 1
	print k
	if i == k:
		return A[q]
	elif i < k:
		return randomized_select(A, p, q, i)
	else:
		return randomized_select(A, q+1, r, i-k)

A = [4,1,5,10,0,20,21]
print randomized_select(A, 0, len(A), 1)


# test_sort(merge_sort)
# test_sort(bubble_sort)
# test_sort(naive_bubble_sort)
# test_sort(insertion_sort)




# test_sort(merge_sort)

# test_sort(qsort)

# print qsort([0, 5, 1, 3, 10])
# test_sort(qsort)
# test_sort(merge_sort)