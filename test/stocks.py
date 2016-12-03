
import numpy as np

stocks = 10 + 5*np.random.randn(100)


def max_profix_brute(stocks):
	max_diff, max_left, max_right = 0, None, None
	for i in xrange(len(stocks)):
		for j in xrange(i, len(stocks)):
			diff = stocks[j] - stocks[i]
			if diff > max_diff:
				max_diff, max_left, max_right = diff, i, j
	return max_diff, max_left, max_right


def max_profit(stocks, lo, hi):
    if hi-lo <= 1:
        return 0

    mp = (hi + lo)/2
    # print "[{} - {} - {}]".format(lo, mp, hi)

    left_best = max_profit(stocks, lo, mp)
    right_best = max_profit(stocks, mp, hi)

    # print "Left best {} [{}], right best {} [{}]".format(left_best, stocks[lo:mp], right_best, stocks[mp:hi])

    min_left = min(stocks[lo:mp])
    max_right = max(stocks[mp:hi])

    # print "Min left {} [{}], max right {} [{}]".format(min_left, stocks[lo:mp], max_right, stocks[mp:hi])

    ans = max(left_best, max(right_best, max_right - min_left))
    # print "Ans: {}".format(ans)
    return ans

print max_profix_brute(stocks)
print max_profit(stocks, 0, len(stocks))




