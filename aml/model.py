

import numpy as np
import logging

from operation import Operation
from feature_pool import FeaturePool
from feature import FeatureType
from config import Config

from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier
from sklearn.neighbors import KNeighborsClassifier
from sklearn.ensemble import RandomForestClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.svm import SVC

from lightgbm import LGBMClassifier
from xgboost import XGBClassifier


logger = logging.getLogger("model")


class Model(Operation):
    def __init__(self, **model_options):
        self.st = Config()
        self.model_options = model_options

    class Output(object):
        def __init__(self, models, fp):
            self.models = models
            self.fp = FeaturePool(fp)

        def __repr__(self):
            return "Model.Output(models=(\n{},), \n\tnum_of_features={})".format(
                "\n".join(["\t{}".format(str(m)) for m in self.models]), len(self.fp)
            )

    @property
    def name(self):
        return self.__class__.__name__

    @staticmethod
    def output_single_model(m, fp):
        return Model.Output((m, ), fp)

    def do(self, fp):
        _, train_x, train_y = fp.train_arrays()

        model = ModelInstance(
            self.name,
            self.create_instance(**self.model_options)
        )
        model = model.train(train_x, train_y)

        return Model.output_single_model(model, fp)

    def create_model(self, **model_options):
        raise NotImplementedError


class ModelInstance(object):
    def __init__(self, name, instance, st=None):
        self.name = name
        self.instance = instance
        self.st = st if st is not None else Config()

    def __repr__(self):
        return "{}(stat={})".format(
            self.name,
            (
                "\n" + "\n".join(["\t{}".format(ss) for ss in str(self.st).split("\n")])
                if len(self.st) > 0 else "{}"
            )
        )

    @property
    def importance(self):
        if hasattr(self.instance, "feature_importances_"):
            return self.instance.feature_importances_.ravel()
        elif hasattr(self.instance, "coef_"):
            return self.instance.coef_.ravel()
        else:
            raise RuntimeError("Feature importance for the model instance `{}` is not supported".format(self.name))

    def train(self, train_x, train_y):
        self.instance.fit(train_x, train_y[:, 0])
        return self

    def predict(self, test_a):
        return self.instance.predict(test_a)

    def predict_prob(self, test_a):
        assert hasattr(self.instance, "predict_proba"), \
            "`predict_proba` method for the model instance `{}` is not supported".format(self.name)
        return self.instance.predict_proba(test_a)

    @staticmethod
    def apply_config(inst, **conf):
        return ModelInstance(
            inst.name,
            inst.instance,
            Config.merge_instances(inst.st, Config.from_dictionary(conf))
        )



class MLogReg(Model):
    def create_instance(self, random_state=None):
        return LogisticRegression(
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





class MPool(Model):
    def __init__(self, *models, **kwargs):
        super(MPool, self).__init__()
        self.models = models

    def do(self, fp):
        _, train_x, train_y = fp.train_arrays()

        def train_models_gen():
            for m in self.models:
                model = ModelInstance(
                    m.name,
                    m.create_instance(**m.model_options)
                )
                yield model.train(train_x, train_y)


        return Model.Output(
            tuple(train_models_gen()),
            fp
        )


class MDecisionTree(Model):
    def create_instance(
        self,
        maximum_depth,
        criterion="gini",
        split_type="best",
    ):
        return DecisionTreeClassifier(
            max_depth=maximum_depth,
            splitter=split_type,
            criterion=criterion,
        )


class MKNNClassifier(Model):
    def create_instance(self):
        return KNeighborsClassifier(
            algorithm='auto',
            leaf_size=10,
            metric='minkowski',
            metric_params=None,
            n_jobs=None,
            n_neighbors=5,
            p=2,
            weights='uniform',
        )


class MRandomForestClassifier(Model):
    def create_instance(self, random_state=None):
        return RandomForestClassifier(
            bootstrap=True,
            class_weight=None,
            criterion='entropy',
            max_depth=3,
            max_features='auto',
            max_leaf_nodes=None,
            min_impurity_decrease=0.0,
            min_impurity_split=None,
            min_samples_leaf=1,
            min_samples_split=2,
            min_weight_fraction_leaf=0.0,
            n_estimators=100,
            n_jobs=None,
            oob_score=False,
            random_state=random_state,
            verbose=0,
            warm_start=False
        )


class MNaiveBayes(Model):
    def create_instance(self):
        return GaussianNB(
            priors=None,
            var_smoothing=1e-09
        )


class MSVC(Model):
    def create_instance(self, random_state=None):
        return SVC(
            C=1.0,
            cache_size=200,
            class_weight=None,
            coef0=0.0,
            decision_function_shape='ovr',
            degree=3,
            gamma=1.0,
            kernel='linear',
            max_iter=-1,
            probability=True,
            random_state=random_state,
            shrinking=True,
            tol=0.001,
            verbose=False
        )




class MLGMBClassifier(Model):
    def create_instance(self, random_state=None):
        return LGBMClassifier(
            boosting_type='gbdt',
            class_weight=None,
            colsample_bytree=1.0,
            learning_rate=0.5,
            max_depth=7,
            min_child_samples=20,
            min_child_weight=0.001,
            min_split_gain=0.0,
            n_estimators=100,
            n_jobs=-1,
            num_leaves=500,
            objective='binary',
            random_state=random_state,
            reg_alpha=0.0,
            reg_lambda=0.0,
            silent=True,
            subsample=1.0,
            subsample_for_bin=200000,
            subsample_freq=0
        )


class MXGBoostClassifier(Model):
    def create_instance(self, random_state=None):
        return XGBClassifier(
            base_score=0.5, booster='gbtree', colsample_bylevel=1,
            colsample_bytree=1, gamma=0, learning_rate=0.9, max_delta_step=0,
            max_depth = 7, min_child_weight=1, missing=None, n_estimators=100,
            n_jobs=1, nthread=None, objective='binary:logistic', random_state=0,
            reg_alpha=0, reg_lambda=1, scale_pos_weight=1, seed=None,
            silent=True, subsample=1
        )



