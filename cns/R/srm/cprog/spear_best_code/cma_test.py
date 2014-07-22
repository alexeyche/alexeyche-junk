import cma
import sys

spearmint_dir = "/home/alexeyche/my/git/spearmint/spearmint/spearmint"

sys.path.append(spearmint_dir)

from spearmint_pb2 import *
from google.protobuf.text_format import Merge as pb_txt_merge

ex = Experiment()
with open("config.pb") as pb_f:
    pb_str = pb_f.read()
pb_txt_merge(pb_str, ex)

## construct an object instance in 4-D, sigma0=1
#es = cma.CMAEvolutionStrategy(4 * [1], 1, {'seed':234})
#id = 0
#while not es.stop():
#   X = es.ask()
#   print X
#   es.tell(X, [cma.fcts.elli(x) for x in X])
#   es.disp()  # by default sparse, see option verb_disp
#
#cma.pprint(es.result())


