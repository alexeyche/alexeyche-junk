
import numpy as np
import pandas as pd
import logging
from collections import OrderedDict
from itertools import chain

from config import Config
from feature import Feature
from feature import FeatureMeta

from sklearn.decomposition import PCA
from matplotlib import pyplot as plt


logger = logging.getLogger("feature_pool")


def gen_feature_pool_from_array(fm, d):
    c_id = 0
    for f in fm:
        yield Feature(f.name, d[:, c_id], f.st)
        c_id += 1
    assert c_id == d.shape[1], \
        "Result feature pool and given array didn't match: {} != {}".format(c_id, d.shape[1])


class FeaturePool(object):

    @staticmethod
    def from_dataframe(df):
        return FeaturePool([Feature.from_series(s) for _, s in df.iteritems()])

    @staticmethod
    def to_test_arrays(fp_list):
        assert isinstance(fp_list, list) or isinstance(fp_list, tuple), \
            "Expecting list or tuple got {}".format(fp_list)

        return FeaturePool(fp_list).test_arrays()

    @staticmethod
    def to_train_arrays(fp_list):
        assert isinstance(fp_list, list) or isinstance(fp_list, tuple), \
            "Expecting list or tuple got {}".format(fp_list)

        return FeaturePool(fp_list).train_arrays()

    @staticmethod
    def from_train_arrays(fm, train_x, train_y):
        return FeaturePool(
            list(chain(
                gen_feature_pool_from_array(fm.train_predictors(), train_x),
                gen_feature_pool_from_array(fm.train_targets(), train_y)
            ))
        )


    @staticmethod
    def from_test_arrays(fm, test_x, test_y):
        return FeaturePool(
            list(chain(
                gen_feature_pool_from_array(fm.test_predictors(), test_x),
                gen_feature_pool_from_array(fm.test_targets(), test_y)
            ))
        )

    def __init__(self, features):
        self.features = features

    def dataframe(self):
        return pd.DataFrame(
            self.array(),
            columns=self.names()
        )

    def array(self):
        assert self.has_data(), "Expecting FeaturePool with data"

        shape = set(self.apply(lambda x: x.data.shape))
        assert len(shape) == 1, \
            "Shape of FeaturePool is not unique, consider different splits: test_split(), train_split()"

        return np.asarray(self.apply(lambda f: f.data)).T


    def test_arrays(self):
        test_x, test_y = self.test_predictors().array(), self.test_targets().array()

        assert len(test_x) > 0, \
            "Test split type is not set. Consider TTrainTestSplit"
        assert len(test_y) > 0, \
            "Target feature type is not set. Consider TSetTarget"
        assert test_y.shape[1] == 1, "Too many targets, support only one"

        return (
            self.test_split().meta(),
            test_x,
            test_y
        )

    def train_arrays(self):
        assert self.has_data(), "Expecting FeaturePool with data"

        train_x, train_y = self.train_predictors().array(), self.train_targets().array()

        assert len(train_x) > 0, \
            "Train split type is not set. Consider TTrainTestSplit"
        assert len(train_y) > 0, \
            "Target feature type is not set. Consider TSetTarget"
        assert train_y.shape[1] == 1, "Too many targets, support only one"


        return (
            self.train_split().meta(),
            train_x,
            train_y
        )

    def apply(self, callback):
        return tuple(
            callback(f)
            for f in self
        )

    def filter(self, callback):
        return FeaturePool([
            f
            for f in self
            if callback(f)
        ])

    def meta(self):
        return FeaturePool(
            self.apply(lambda f: FeatureMeta(f.name, f.st))
        )

    def predictors(self):
        return self.filter(lambda f: f.is_predictor)

    def targets(self):
        return self.filter(lambda f: f.is_target)

    def test_split(self):
        return self.filter(lambda f: f.is_test)

    def train_split(self):
        return self.filter(lambda f: f.is_train)

    def train_predictors(self):
        return self.filter(lambda f: f.is_train and f.is_predictor)

    def test_predictors(self):
        return self.filter(lambda f: f.is_test and f.is_predictor)

    def train_targets(self):
        return self.filter(lambda f: f.is_train and f.is_target)

    def test_targets(self):
        return self.filter(lambda f: f.is_test and f.is_target)

    def names(self):
        return self.apply(lambda f: f.name)

    def has_data(self):
        return all(self.apply(lambda f: f.has_data))

    def __len__(self):
        return len(self.features)

    def __iter__(self):
        return iter(self.features)

    def __getitem__(self, idx):
        if isinstance(idx, basestring):
            r = [f for f in self.features if f.name == idx]
            assert len(r) > 0, "Feature name not found: `{}`".format(idx)
            assert len(r) == 1, "Found duplicate names for: `{}`".format(idx)
            return r[0]
        return self.features[idx]

    def __repr__(self):
        return "FeaturePool(\n\tfeatures_size={}, \n\tnames=\n{}\n)".format(
            len(self.features),
            "\n".join(["\t\t{}".format(f.name) for f in self.features]),
        )

    def pca(self, plot=True, split_by=None):
        assert self.has_data(), "Expecting FeaturePool with data"

        assert plot or split_by is None, \
            "`split_by` option doesn't make sense without `plot` == True"
        pca = PCA(n_components = 2)

        if split_by is not None:
            X = FeaturePool([f for f in self.features if f.name != split_by]).array()
        else:
            X = self.array()

        pc = pca.fit_transform(X)
        if plot:
            fig = plt.figure(figsize=(7, 7))
            ax = fig.add_subplot(111)
            if split_by is not None:
                d = self[split_by].data
                ax.scatter(pc[:, 0], pc[:, 1], c=d)
            else:
                ax.scatter(pc[:, 0], pc[:, 1])
            if split_by is not None:
                ax.set_title(
                    "PCA for a feature pool splitted by feature ``".format(split_by)
                )
            else:
                ax.set_title(
                    "PCA for a feature pool"
                )
            fig.show()
        return pc

    def corr(self, plot=True):
        assert self.has_data(), "Expecting FeaturePool with data"

        corrmat = self.dataframe().corr()

        if plot:
            import seaborn as sns

            f = plt.figure(figsize=(7, 7))
            g = sns.heatmap(corrmat, vmax=.8, square=True)
            g.set_yticklabels(g.get_yticklabels(), rotation = 'horizontal', fontsize = 8)
            g.set_xticklabels(g.get_xticklabels(), rotation = 'vertical', fontsize = 8)
            f.show()

        return corrmat
