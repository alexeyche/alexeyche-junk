

ar = [3, 1, 2, 5, 6, 4]

ra = [1, 3, 3*1, 3*1*2, 3*1*2*5, 3*1*2*5*6]


ra = [1, 3, 3*1, 3*1*2, 3*1*2*5, 3*1*2*5*6]

ra = [None]*len(ar)


def mult(ar, ra, fr, to, step):
	for i in xrange(fr, to, step):
		if i > 0:
			prev = ra[i-1]
			# print i, "{} * {}".format(ar[i-1], prev)
			ra[i] = "{} * {}".format(ar[i-1], prev)
		# else:
			# ra[i] = "1"
	return ra



ra = ["one"]*len(ar)
ra = mult(ar, ra, 0, len(ar), 1)

ra2 = ["one"]*len(ar)
ra2 = mult(list(reversed(ar)), ra2, 0, len(ar), 1)
print ra
print list(reversed(ra2))
# ra = mult(ar, ra, len(ar)-1, -1, -1)




[60, 65, 40, 31, 75, 20]
