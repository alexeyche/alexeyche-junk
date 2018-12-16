

from collections import deque
import types

from feature_pool import FeaturePool
from transform import Transform
from validation import Validation
from model import Model
from metric import Metric


class Pipeline(object):
    def __init__(self, *ops):
        self.ops = deque(ops)


    def run(self, d):
        while len(self.ops) > 0:
            op = self.ops.popleft()
            if isinstance(op, Transform):
                assert isinstance(d, FeaturePool), \
                    "Expecting `FeaturePool`, got {}".format(d)

                d = FeaturePool([f for f in op(d.features)])

            elif isinstance(op, Model):
                assert isinstance(d, FeaturePool), \
                    "Expecting `FeaturePool`, got {}".format(d)

                d = op(d)

            elif isinstance(op, Validation):
                assert isinstance(d, Model.Output), \
                    "Expecting Model.Output for validation, got {}".format(d)

                d = op(d)
            else:
                raise ValueError("Failed to dispatch operation: `{}`".format(op))
        return d
