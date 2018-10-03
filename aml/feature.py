
import numpy as np

import pandas as pd
from matplotlib import pyplot as plt
import re
import itertools
from config import Config
import logging
import sys
import pprint

NAN_RATIO_UPPER_BOUND = 0.25
FLOAT_PRECISION = np.float64
CATEGORY_SIZE_UPPER_BOUND = 50


logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
logger = logging.getLogger("feature")

def dformat(d):
    return pprint.pformat(d, width=1)


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

def feature_preprocess(data):
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
        cardinal_regex = re.compile("^\d+?$")  # TODO
        
        v = s.apply(lambda x: not float_regex.match(x) is None)
        vh = v.value_counts()

        float_parse_count = vh[True] if True in vh else 0
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

    else:
        raise ValueError("Unable to recognize numpy type: {}".format(data.dtype))



class Feature(object):

    @staticmethod
    def from_series(series):
        return Feature(series.name, series.values)

    def __init__(self, name, data):
        assert len(data.shape) == 1, "Data should be one dimensional"
        self.name = name
        self.data, self.st = feature_preprocess(data)
        summary_st = feature_summary(self.data)
        self.st.update(summary_st)

    def __getattr__(self, n):
        if n in self.__dict__: return self.__dict__[n]
        if n in self.st: return self.st[n]
        raise KeyError("Failed to find attribute: `{}`".format(n))

    @property
    def data_size(self):
        return self.data.shape[0]

    def __repr__(self):
        return "Feature(name={}, data_size={}, stat=\n{})".format(
            self.name, 
            self.data.shape[0],
            "\n".join(["\t{}".format(ss) for ss in str(self.st).split("\n")])
        )

    def plot(self, split_by=None):
        fig = plt.figure(figsize=(7, 7))
        ax = fig.add_subplot(111)

        if not split_by is None:
            assert split_by.categorical, \
                "Split by feature `{}` should be categorical one".format(split_by.name)

            if self.categorical: 
                cats = sorted(self.cats.keys())
                np.sum(self.cats.values())
                ind = np.arange(len(cats))
                inc_width = 0.0
                width = 0.1
                rects_arr = []
                for cat in split_by.cats:
                    idx = np.where(split_by.data == cat)
                    
                    unique, counts = np.unique(self.data[idx], return_counts=True)
                    split_hist = dict([
                        (u, c / float(np.sum(counts))) 
                        for (u, c) in zip(unique, counts)
                    ])
                    logger.info(
                        "Feature({}): Got split histogram by `{}` = `{}`:\n{}".format(
                            self.name, split_by.name, cat, dformat(split_hist)
                        )
                    )
                    res_count = []
                    for c in cats:
                        cat_count = split_hist[c] if c in split_hist else 0
                        res_count.append(cat_count)

                    rects = ax.bar(ind + inc_width, res_count, width)

                    rects_arr.append(rects[0])

                    inc_width += width
                ax.set_xticklabels(cats)
                ax.set_xticks(ind + width / len(cats))
                ax.legend(tuple("{}: {}".format(split_by.name, c) for c in split_by.cats))
            else:
                data = []
                for c in split_by.cats:
                    idx = np.where(split_by.data == c)
                    data.append(self.data[idx])

                ax.hist(
                    data, 
                    bins="auto", 
                    normed=True, 
                    label=[
                        "{}: {}".format(split_by.name, k)
                        for k in split_by.cats.keys()
                    ]
                )
                ax.legend()
            
            ax.set_title(
                "Histogram side by side plot, `{}` split by `{}`".format(self.name, split_by.name)
            )
            ax.set_xlabel(self.name)
            ax.set_ylabel("Normalized `{}`".format(self.name))
        else:
            if self.categorical: 
                cats = sorted(self.cats.keys())
                total = np.sum(self.cats.values())
                ind = np.arange(len(cats))
                inc_width = 0.0
                width = 0.1
                hist = dict([(n, self.cats[n] / float(total)) for n in cats])

                logger.info("Feature({}): Got histogram:\n{}".format(self.name, dformat(hist)))
                rects = ax.bar(ind + inc_width, hist.values(), width)

                ax.set_xticklabels(cats)
                ax.set_xticks(ind + width / len(cats))
            else:
                ax.hist(
                    self.data, 
                    bins="auto", 
                    normed=True
                )
                ax.set_title(
                    "Histogram for a feature `{}`".format(self.name)
                )
                ax.set_xlabel(self.name)
                ax.set_ylabel("Normalized `{}`".format(self.name))
        fig.show()


def feature_pool_preprocess(features):
    st = Config()
    not_nan_mask = pd.Series(features[0].data).notnull()
    for f in features:
        not_nan_mask &= pd.Series(f.data).notnull()

    not_nan_hist = not_nan_mask.value_counts()
    
    non_nan_count = not_nan_hist[True] if True in not_nan_hist else 0
    nan_count = not_nan_hist[False] if False in not_nan_hist else 0
    st.missing_data_ratio = nan_count / float(nan_count + non_nan_count)
    
    if st.missing_data_ratio >= NAN_RATIO_UPPER_BOUND:
        raise ValueError("Ratio of NaN values is too high: {}".format(st.missing_data_ratio))
    
    for f in features:
        f.data = f.data[not_nan_mask]
    return features, st

class FeaturePool(object):
    
    @staticmethod
    def from_dataframe(df):
        return FeaturePool([Feature.from_series(s) for _, s in df.iteritems()])

    def __init__(self, features):
        self.features, self.st = feature_pool_preprocess(features)

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
        return "FeaturePool(\n\tfeatures_size={}, \n\tnames=\n{}\n\tstat=\n\t{})".format(
            len(self.features),
            "\n".join(["\t\t{}".format(f.name) for f in self.features]),
            "\n".join(["\t{}".format(ss) for ss in str(self.st).split("\n")])
        )

