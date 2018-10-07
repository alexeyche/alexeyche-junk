
from collections import deque
from feature_pool import FeaturePool
from transform import Transform
from model import Model
from metric import Metric

class Pipeline(object):
    def __init__(self, *ops):
        self.ops = deque(ops)


    def run(self, d):
        while len(self.ops) > 0:
            op = self.ops.popleft()
            if isinstance(op, Transform):
                assert isinstance(d, FeaturePool), "Expecting `FeaturePool`"
                
                d = FeaturePool([f for f in op(d.features)])

            elif isinstance(op, Model):
                assert isinstance(d, FeaturePool), "Expecting `FeaturePool`"

                d = op(d.features)

            else:
                raise ValueError("Failed to dispatch operation: `{}`".format(op))
        return d
