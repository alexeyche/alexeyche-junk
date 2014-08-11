import cma
import sys

# construct an object instance in 4-D, sigma0=1
es = cma.CMAEvolutionStrategy(4 * [1], 1, {'seed':234})
id = 0
while not es.stop():
   X = es.ask()
   print X
   es.tell(X, [cma.fcts.elli(x) for x in X])
   es.disp()  # by default sparse, see option verb_disp

cma.pprint(es.result())


