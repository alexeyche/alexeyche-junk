
import numpy as np

import pandas as pd
from matplotlib import pyplot as plt
import re
import itertools
import logging
import sys
import pprint
from functools import wraps
import collections
from collections import OrderedDict

from feature import Feature
from feature import SplitType
from feature import FeatureType
from feature_pool import FeaturePool
from operation import Operation
from config import Config
from plot_utils import plot_importance

from sklearn.preprocessing import LabelEncoder
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from imblearn.over_sampling import SMOTE
from sklearn.feature_selection import RFE
from sklearn.feature_selection import chi2
from sklearn.feature_selection import SelectKBest as SKLearnSelectKBest
from sklearn.preprocessing import MinMaxScaler
from umap import UMAP

NAN_RATIO_UPPER_BOUND = 0.25
FLOAT_PRECISION = np.float64
CATEGORY_SIZE_UPPER_BOUND = 50


logger = logging.getLogger("transform")


def feature_parse(data):
    assert isinstance(data, np.ndarray), "Data should be ndarray"

    def process_nan(s, st):
        not_nan_hist = s.notnull().value_counts()
        non_nan_count = not_nan_hist[True] if True in not_nan_hist else 0
        nan_count = not_nan_hist[False] if False in not_nan_hist else 0
        st.missing_data_ratio = nan_count / float(nan_count + non_nan_count)

        if st.missing_data_ratio >= NAN_RATIO_UPPER_BOUND:
            raise ValueError("Ratio of NaN values is too high: {}".format(st.missing_data_ratio))
        return s, st

    s = pd.Series(data)
    st = Config()
    if data.dtype in set((np.dtype('O'), np.dtype('S'), np.dtype('U'))):
        float_regex = re.compile("^\d+?\.\d+?$")
        # cardinal_regex = re.compile("^\d+?$")  # TODO

        v = s.apply(lambda x: not float_regex.match(x) is None)
        vh = v.value_counts()

        # float_parse_count = vh[True] if True in vh else 0
        non_float_parse_count = vh[False] if False in vh else 0

        ratio_non_float = non_float_parse_count / float(v.shape[0])

        if ratio_non_float > 0.95:
            # Data looks like a non float
            sf = s.apply(lambda x: np.nan if not float_regex.match(x) is None else str(x))
            s, st = process_nan(s, st)  # ??, should empty string? np.nan is a float
            max_string_size = s.apply(lambda x: len(x)).max()
            st.type = np.dtype("|S{}".format(max_string_size))
            return (
                sf.values,
                st
            )

        elif ratio_non_float > 0.75:
            raise ValueError("Data is very dirty: 3/4 is non float parsable data")

        else:
            s = s.apply(lambda x: np.nan if float_regex.match(x) is None else float(x))
            st.type = np.dtype(FLOAT_PRECISION)
            s, st = process_nan(s, st)
            return (
                s.values.astype(FLOAT_PRECISION),
                st
            )

    elif data.dtype in set((np.dtype("i"), np.dtype("int64"),)):
        s, st = process_nan(s, st)
        st.type = np.dtype(np.dtype("int64"))
        return (
            s.values.astype(FLOAT_PRECISION),
            st
        )

    elif data.dtype in set((np.dtype("f"), np.dtype("float64"))):
        s, st = process_nan(s, st)
        st.type = np.dtype(FLOAT_PRECISION)
        return (
            s.values.astype(FLOAT_PRECISION),
            st
        )
    elif data.dtype in set((np.dtype("uint8"),)):
        s, st = process_nan(s, st)
        st.type = np.dtype(np.dtype("uint8"))
        return (
            s.values.astype(FLOAT_PRECISION),
            st
        )
    else:
        raise ValueError("Unable to recognize numpy type: {}".format(data.dtype))




def feature_summary(data):
    assert isinstance(data, np.ndarray), "Data should be ndarray"
    st = Config()
    unique, counts = np.unique(data, return_counts=True)

    if len(unique) < CATEGORY_SIZE_UPPER_BOUND:
        cats = dict([(cat, c)for cat, c in zip(unique, counts)])

        st.summary = Config.from_dictionary(pd.Series(data).describe().to_dict())

        st.cats = cats
        st.categorical = True
    else:
        st.summary = Config.from_dictionary(pd.Series(data).describe().to_dict())
        st.cats = None
        st.categorical = False

    return st




def expected_feature(categorical, n_cats=None):
    def expected_feature_decorator(func):
        @wraps(func)
        def wrapper(self, f):
            assert not categorical or f.categorical, \
                "Expected categorical feature"
            assert n_cats is None or len(f.cats) == n_cats, \
                "Expected {} categories".format(n_cats)
            return func(self, f)
        return wrapper
    return expected_feature_decorator


class Transform(Operation):
    def transform(self, f):
        assert isinstance(f, collections.Iterable), \
            "Transform should work only with iterable input"

        for ff in f:
            yield self.transform_single(ff)

    def transform_single(self, f):
        raise NotImplementedError

    def do(self, f):
        return self.transform(f)



class TParse(Transform):
    def transform_single(self, f):
        data, st = feature_parse(f.data)
        return Feature.merge_instances(
            f,
            Feature(f.name, data, st)
        )


class TSummary(Transform):
    def transform_single(self, f):
        st = feature_summary(f.data)

        return Feature.merge_instances(
            f,
            Feature(f.name, f.data, st)
        )


class TClean(Transform):
    def __init__(self, positive_mask):
        self.positive_mask = positive_mask

    def transform_single(self, f):
        return Feature.merge_instances(
            f,
            Feature(f.name, f.data[self.positive_mask])
        )



class TStandardScaler(Transform):
    @expected_feature(categorical=False)
    def transform_single(self, f):
        std = StandardScaler()
        one_dim_f = len(f.data.shape) == 1
        res_data = std.fit_transform(
            f.data.reshape(-1, 1) if one_dim_f else f.data
        )
        if one_dim_f:
            res_data = res_data.reshape(-1)

        return Feature.merge_instances(
            f,
            Feature(f.name, res_data)
        )

class TIdentity(Transform):
    def transform_single(self, f):
        return f


### Pool transforms




def feature_pool_clean(features):
    st = Config()
    not_nan_mask = pd.Series(features[0].data).notnull()
    for f in features:
        not_nan_mask &= pd.Series(f.data).notnull()

    not_nan_hist = not_nan_mask.value_counts()

    non_nan_count = not_nan_hist[True] if True in not_nan_hist else 0
    nan_count = not_nan_hist[False] if False in not_nan_hist else 0
    st.pool_missing_data_ratio = nan_count / float(nan_count + non_nan_count)

    if st.pool_missing_data_ratio >= NAN_RATIO_UPPER_BOUND:
        raise ValueError("Ratio of NaN values is too high: {}".format(st.pool_missing_data_ratio))

    logger.info("feature_pool_clean(): Missing data ratio {:.4f}".format(st.pool_missing_data_ratio))

    for f in features:
        f.st.update(st)
    return TClean(not_nan_mask).transform(features)


def one_hot_encode(f):
    res_data = np.zeros((f.data.shape[0], len(f.cats)), dtype=FLOAT_PRECISION)
    cats = dict([(c, c_id) for c_id, c in enumerate(sorted(f.cats))])
    cat_ids = pd.Series(f.data).apply(lambda r: cats.get(r))
    res_data[np.arange(f.data.shape[0]), cat_ids] = 1.0
    fp = [
        Feature(
            f.name + "_{}".format(c),
            res_data[:, c_id]
        )
        for c, c_id in cats.items()
    ]
    return fp



class TCleanPool(Transform):
    def transform(self, f):
        res_f = feature_pool_clean(list(f))
        for ff in res_f:
            yield ff


class TLabelEncoder(Transform):
    def transform(self, fp):
        for f in fp:
            if f.categorical and len(f.cats) > 2: # categorical
                yield self.transform_single(f)
            else:
                yield f

    @expected_feature(categorical=True)
    def transform_single(self, f):
        le = LabelEncoder()
        return Feature.merge_instances(
            f,
            Feature(f.name, le.fit_transform(f.data))
        )



class TPreprocessPool(Transform):
    def transform(self, fp):
        for f in fp:
            assert not f.categorical or len(f.cats) >= 2, "Found less than 2 categories: {}".format(f)
            if f.categorical and len(f.cats) == 2: # binary
                yield TLabelEncoder().transform_single(f)

            elif f.categorical and len(f.cats) > 2: # categorical
                for hot_f in one_hot_encode(f):
                    yield hot_f
            else:
                assert not f.categorical, "UB"  # continious
                if f.type.kind == 'S':
                    # skipping a key
                    logger.info("TPreprocessPool: skipping key `{}`".format(f.name))
                else:
                    yield TStandardScaler().transform_single(f)



def feature_pool_to_array(fp, st=None, ft=None):
    return np.asarray([
        f.data
        for f in fp
        if (st is None or st == f.split_type) and
           (ft is None or ft == f.feature_type)
    ]).T

class TCleanRedundantFeatures(Transform):
    def __init__(self, correlation_bound=0.9999):
        self.correlation_bound = correlation_bound

    def transform(self, fp):
        m = pd.DataFrame(
            feature_pool_to_array(fp), columns=[f.name for f in fp]
        ).corr()

        np.fill_diagonal(m.values, 0.0)

        def corr_f(pred):
            redundant_f = set(
                np.unique(m.columns.values[np.where(pred(m.values))[0]])
            )
            left = None
            if len(redundant_f) > 0:
                left = redundant_f.pop() # Randomly leaving feature
            return redundant_f, left

        # TODO: make it just |1.0|
        r_pos_set, pos_left = corr_f(lambda x: x >= self.correlation_bound)
        r_neg_set, neg_left = corr_f(lambda x: x <= -self.correlation_bound)
        for f in fp:
            if f.name in r_pos_set | r_neg_set:
                comment_str = ""
                if pos_left:
                    comment_str += ", leaving `{}`".format(pos_left)
                if neg_left:
                    comment_str += ", leaving `{}`".format(neg_left)

                logger.warn(
                    "Getting rid of redudant feature (correlation is almost |1.0|): {}{}".format(
                        f.name,
                        comment_str
                    )
                )

            else:
                yield f

class TTrainTestSplit(Transform):
    def __init__(self, test_size = 0.25, random_state=None):
        self.test_size = test_size
        self.random_state = random_state

    def transform(self, fp):
        train_a, test_a = train_test_split(
            FeaturePool(fp).array(),
            test_size = self.test_size,
            random_state = self.random_state,
        )

        for f_id, f in enumerate(fp):
            yield Feature.apply_config(
                Feature(f.name, train_a[:, f_id], f.st),
                split_type=SplitType.TRAIN
            )

        for f_id, f in enumerate(fp):
            yield Feature.apply_config(
                Feature(f.name, test_a[:, f_id], f.st),
                split_type=SplitType.TEST
            )


class TSetTarget(Transform):
    def __init__(self, name):
        self.name = name

    def transform(self, fp):
        found_feature = False
        for f in fp:
            if f.name == self.name:
                yield Feature.apply_config(
                    f,
                    feature_type=FeatureType.TARGET
                )
                found_feature = True
            else:
                yield Feature.apply_config(
                    f,
                    feature_type=FeatureType.PREDICTOR
                )
        assert found_feature, "Feature `{}` is not found in the FeaturePool".format(self.name)

class TOverSampling(Transform):
    def __init__(self, random_state = 0):
        self.random_state = random_state

    def transform(self, fp):
        fm, train_x, train_y = FeaturePool.to_train_arrays(fp)

        os = SMOTE(random_state = self.random_state)
        os_train_x, os_train_y = os.fit_sample(train_x, train_y[:, 0])
        os_train_y = os_train_y.reshape((os_train_y.shape[0], 1))

        for f in FeaturePool.from_train_arrays(fm, os_train_x, os_train_y):
            yield Feature.apply_config(f, is_over_sampled=True)
        for f in fp:
            if f.split_type == SplitType.TEST:
                yield f






class TFeatureElimination(Transform):
    def __init__(self, model, num_of_features):
        self.model = model
        self.num_of_features = num_of_features
        self._inst = RFE(
            self.model.create_instance(**self.model.model_options),
            self.num_of_features
        )

    def fit_model(self, fp):
        fm, train_x, train_y = FeaturePool.to_train_arrays(fp)
        train_fm = fm.predictors()

        m = self._inst.fit(train_x, train_y.ravel())

        assert len(train_fm) == len(m.support_), \
            "Size of output of RFE does not equals to the metadata {} != {}".format(
                len(train_fm),
                len(m.support_)
            )
        return m

    def transform(self, fp):
        m = self.fit_model(fp)
        supp = set([
            f.name
            for f, support in zip(FeaturePool(fp).train_predictors(), m.support_)
            if support
        ])

        for f in fp:
            if f.is_predictor:
                if f.name in supp:
                    yield f
                else:
                    logger.info("TFeatureElimination: eliminating feature `{}`".format(f.name))
            else:
                yield f

class TSelectKBest(Transform):
    def __init__(self, num_of_features):
        self._inst = SKLearnSelectKBest(
            score_func=chi2,
            k=num_of_features
        )

    def fit_model(self, fp):
        fp = FeaturePool(fp)
        p = (
            fp.train_split()
              .predictors()
              .filter(lambda f: f.categorical)
        )
        x = p.array()
        y = (
            fp.train_split()
              .targets()
              .array()
        )

        return self._inst.fit(x, y)

    def plot(self, fp):
        m = self.fit_model(fp)

        preds = np.asarray(
            fp.train_predictors()
              .filter(lambda f: f.categorical)
              .names()
        )

        plot_importance(m.scores_, preds)


    def transform(self, fp):
        m = self.fit_model(fp)
        p = (
            fp.train_split()
              .predictors()
              .filter(lambda f: f.categorical)
        )
        supp = set([
            f.name
            for f, support in zip(p, m.get_support())
            if support
        ])

        for f in fp:
            if f.is_predictor:
                if f.name in supp:
                    yield f
                else:
                    logger.info("TSelectKBest: eliminating feature `{}`".format(f.name))
            else:
                yield f


class TLambda(Transform):
    def __init__(self, callback):
        self.callback = callback

    def transform(self, fp):
        self.callback(FeaturePool(fp))
        for f in fp:
            yield f

class TMinMaxScaler(Transform):
    def __init__(self, feature_range):
        self.feature_range = feature_range

    def transform(self, fp):
        fm = FeaturePool(fp).meta()
        x = FeaturePool(fp).array()

        scaler = MinMaxScaler(feature_range = self.feature_range)
        scaler.fit(x)
        for f in FeaturePool.from_array(fm, scaler.transform(x)):
            yield f


class TUmap(Transform):
    """
    n_neighbors:
        This determines the number of neighboring points used in local approximations
        of manifold structure. Larger values will result in more global structure being
        preserved at the loss of detailed local structure.
        In general this parameter should often be in the range 5 to 50,
        with a choice of 10 to 15 being a sensible default.
    min_dist:
        This controls how tightly the embedding is allowed compress points together.
        Larger values ensure embedded points are more evenly distributed, while smaller
        values allow the algorithm to optimise more accurately with regard to local structure.
        Sensible values are in the range 0.001 to 0.5, with 0.1 being a reasonable default.
    metric:
        This determines the choice of metric used to measure distance in the input space.
        A wide variety of metrics are already coded, and a user defined function can be passed
        as long as it has been JITd by numba.
    """

    def __init__(
        self,
        n_neighbors=15,
        min_dist=0.1,
        metric="euclidean",
        n_components=2,
        spread=1.0,
        random_state=None
    ):
        self._inst = UMAP(
            n_neighbors = n_neighbors,
            min_dist = min_dist,
            metric = metric,
            n_components=n_components,
            spread=spread,
        )


    def transform(self, fp):
        x = FeaturePool(fp).array()
        logger.info("TUmap: starting UMAP transform ...")
        x_emb = self._inst.fit_transform(x)
        logger.info("TUamp: Done")

        for f_id in range(x_emb.shape[1]):
            yield Feature(
                "UMAP feature #{}".format(f_id),
                x_emb[:, f_id]
            )

    @staticmethod
    def plot_embedding(efp: FeaturePool, split_by=None):
        x = efp.array()
        assert x.shape[1] == 2, "Embedding is expected to be with the size 2 to plot, got {}".format(x.shape[1])
        fig = plt.figure(figsize=(7, 7))
        ax = fig.add_subplot(111)
        if split_by is not None:
            d = split_by.data
            ax.scatter(x[:, 0], x[:, 1], c=d, alpha=0.5)
        else:
            ax.scatter(x[:, 0], x[:, 1], alpha=0.5)
        if split_by is not None:
            ax.set_title(
                "UMAP for a feature pool splitted by feature `{}`".format(split_by.name)
            )
        else:
            ax.set_title(
                "UMAP for a feature pool"
            )
        fig.show()
























