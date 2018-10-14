

import numpy as np
import logging

from operation import Operation
from feature_pool import FeaturePool
from feature import FeatureType

from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier

logger = logging.getLogger("model")


class Model(Operation):
    class Output(object):
        def __init__(self, models, fp):
            self.models = models
            self.fp = FeaturePool(fp)

        def __repr__(self):
            return "Model.Output(models=({},), num_of_features={})".format(
                ", ".join([str(m) for m in self.models]), len(self.fp)
            )

    @property
    def name(self):
        return self.__class__.__name__

    @staticmethod
    def output_single_model(m, fp):
        return Model.Output((m, ), fp)

    def do(self, fp):
        _, train_x, train_y = fp.train_arrays()
        model = self.train(train_x, train_y)

        return Model.output_single_model(model, fp)

    def train(self, train_x, train_y):
        """
        Returns:
            model_instance generator
        """
        raise NotImplementedError


class MPool(Model):
    def __init__(self, *models, **kwargs):
        self.models = models

    def do(self, fp):
        _, train_x, train_y = fp.train_arrays()

        return Model.Output(
            [
                m.train(train_x, train_y)
                for m in self.models
            ],
            fp
        )


class MLogReg(Model):
    def __init__(self, random_state=None):
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
            random_state=random_state,
            solver='newton-cg',
            tol=0.0001,
            verbose=1,
            warm_start=False
        )

    @property
    def importance(self):
        return self._inst.coef_.ravel()


    def train(self, train_x, train_y):
        self._inst.fit(train_x, train_y[:, 0])
        return self


    def predict(self, test_a):
        return self._inst.predict(test_a)

    def predict_prob(self, test_a):
        return self._inst.predict_proba(test_a)



class MDecisionTree(Model):
    def __init__(
        self,
        maximum_depth,
        criterion="gini",
        split_type="best",
    ):
        self._inst = DecisionTreeClassifier(
            max_depth=maximum_depth,
            splitter=split_type,
            criterion=criterion,
        )
        self._column_names = None
        self._target_name = None

    @property
    def importance(self):
        return self._inst.feature_importances_.ravel()

    def do(self, fp):
        fm, train_x, train_y = FeaturePool.to_train_arrays(fp)

        # for plotting mostly
        self._column_names = [
            f.name
            for f in fm
            if f.feature_type == FeatureType.PREDICTOR
        ]
        target_names = [
            f.name
            for f in fm
            if f.feature_type == FeatureType.TARGET
        ]
        assert len(target_names) == 1
        self._target_name = target_names[0]
        ##############

        model = self.train(train_x, train_y)

        return Model.output_single_model(model, fp)


    def train(self, train_x, train_y):
        self._inst.fit(train_x, train_y[:, 0])
        return self

    def predict(self, test_a):
        return self._inst.predict(test_a)

    def predict_prob(self, test_a):
        return self._inst.predict_proba(test_a)

    def plot(self):
        # veery dirty, didn't find another way :C
        from sklearn.tree import export_graphviz
        from sklearn import tree
        from graphviz import Source
        import pydot
        from StringIO import StringIO
        import subprocess as sub
        import tempfile

        assert self._column_names is not None
        assert self._target_name is not None

        new_file, filename = tempfile.mkstemp(suffix=".png")

        dotfile = StringIO()

        tree.export_graphviz(
            self._inst,
            out_file=dotfile,
            rounded=True,
            proportion = False,
            feature_names = self._column_names,
            precision = 2,
            class_names = [
                "Not {}".format(self._target_name),
                self._target_name
            ],
            filled = True
        )
        v = pydot.graph_from_dot_data(dotfile.getvalue())[0]
        v.write_png(filename)
        sub.call(["open", filename])


