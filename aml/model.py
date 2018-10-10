

import numpy as np
import logging

from operation import Operation
from feature_pool import FeaturePool

from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split

logger = logging.getLogger("model")

class Split(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def __repr__(self):
        return "Split(x={}, y={})".format(self.x.shape, self.y.shape)



class Model(Operation):
    class Output(object):
        def __init__(self, model, train, test):
            self.model = model
            self.train = train
            self.test = test


    DEFAULT_TEST_SIZE = 0.25

    def __init__(self, target, test_size):
        self.test_size = test_size
        self.target = target

    @staticmethod
    def split_features(_f, target, test_size):
        fp = FeaturePool(_f)

        train_a, test_a = train_test_split(
            FeaturePool.to_array(fp),
            test_size = test_size
        )

        cid_x = [f_id for f_id, f in enumerate(fp) if f.name != target]
        cid_y = [ft_id for ft_id, ft in enumerate(fp) if ft.name == target]

        assert len(cid_y) > 0, \
            "Failed to find target feature with name `{}`".format(target)
        assert len(cid_x) > 0, \
            "Train data is empty"

        train = Split(train_a[:, cid_x], train_a[:, cid_y])
        test = Split(test_a[:, cid_x], test_a[:, cid_y])
        return train, test
        
    def do(self, f):
        """
        Returns:
            gen(model_instance, train_features, test_features)
        """
        train, test = Model.split_features(f, self.target, self.test_size)
        
        model = self.train(train)

        yield Model.Output(model, train, test)

    def train(self, train):
        """
        Returns:
            model_instance generator
        """
        raise NotImplementedError


class MPool(Model):
    def __init__(self, *models, **kwargs):
        self.models = models
        targets = set([m.target for m in self.models])
        assert len(targets) == 1, \
            "Need one unique target to work with models pool"

        target = targets.pop()

        super(MPool, self).__init__(
            test_size = kwargs.get("test_size", Model.DEFAULT_TEST_SIZE),
            target = target
        )



    def do(self, f):
        """
        Returns:
            (model_instance, train_features, test_features)
        """
        train, test = Model.split_features(f, self.target, self.test_size)
        
        for m in self.models:
            model = m.train(train)
            yield Model.Output(model, train, test)



class MLogReg(Model):
    def __init__(self, target, test_size=Model.DEFAULT_TEST_SIZE):
        super(MLogReg, self).__init__(
            target = target,
            test_size = test_size
        )

        self._inst = LogisticRegression(
            C=1.0,  # Like in support vector machines, smaller values specify stronger regularization.
            class_weight=None,
            dual=False,
            fit_intercept=True,
            intercept_scaling=1,
            max_iter=100,
            multi_class='ovr',
            n_jobs=None,
            penalty='l2',
            random_state=None,
            solver='newton-cg',
            tol=0.0001,
            verbose=1,
            warm_start=False
        )

    def importance(self):
        return self._inst.coef_.ravel()


    def train(self, train):
        self._inst.fit(train.x, train.y[:, 0])
        return self


    def predict(self, test_a):
        return self._inst.predict(test_a)

    def predict_prob(self, test_a):
        return self._inst.predict_proba(test_a)




