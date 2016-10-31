


def gen_par(s, i, n):
	if len(s) == n:
		if i == 0:
			yield s
		return
		
	for subs in gen_par(s+"(", i+1, n):
		yield subs

	if i > 0:
		for subs in gen_par(s+")", i-1, n):
			yield subs


for s in gen_par("", 0, 12):
	print s

