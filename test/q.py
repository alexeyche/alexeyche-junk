

import random



def swap(arr, i, j):
	tmp = arr[j]
	arr[j] = arr[i]
	arr[i] = tmp


def knuth_shuffle(arr):
	for i in xrange(len(arr)):
		num = random.randint(i, len(arr)-1)
		swap(arr, i, num)
	return arr


ks = knuth_shuffle(range(-20/2, 20/2))

def max_subarray_sum(series):
	cum_sum = 0
	max_cum_sum = 0

	for i in xrange(len(series)):
		cum_sum = max(cum_sum + series[i], 0)
		max_cum_sum = max(max_cum_sum, cum_sum)
	return max_cum_sum

# series = [-100, 1,2,3,-10, -9,10, 20, -100]

# print max_subarray_sum(series)

def disjoint_set_sum(given_set):
	given_set_sum = sum(given_set)
	if len(given_set) % 2 != 0 and given_set_sum % 2 != 0:
		return False




def generate_all_combinations(given_set, need_empty_set=True):
	if len(given_set) == 0:
		return
	if len(given_set) == 1:
		yield given_set
		return

	head, tail = given_set[0], given_set[1:]
	if need_empty_set:
		yield ""
	yield head
	for v in generate_all_combinations(tail, False):
		yield v
	for v in generate_all_combinations(tail, False):
		yield head + v
	

from itertools import combinations, chain


def all_subsets(ss):
 	return chain(*map(lambda x: combinations(ss, x), range(0, len(ss)+1)))


# stuff = "abcdef"
# subsets = [ "".join(subset) for subset in all_subsets(stuff) ]
 	
# subsets_my = list(generate_all_combinations(stuff))
# print subsets
# print subsets_my

# print list(generate_all_combinations([1,2,3]))
# print list(chain(*[combinations([1,2,3],1), combinations([1,2,3],2), combinations([1,2,3],3)]))


# s = "abc"

# def convert_to_binary(dec, rep_len):
# 	rem = dec
# 	rep = []
# 	while rem > 0:
# 		rep.append(rem % 2)
# 		rem = rem / 2
# 	while len(rep) < rep_len:
# 		rep = [0] + rep
# 	return rep

# for idx in xrange(2 ** len(s)):
# 	print convert_to_binary(idx, 3)
# 	# cum_div = s[0]
# 	# for sub_idx in xrange(1, len(s)):
# 	# 	print idx % 2
	
