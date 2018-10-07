
import numpy as np

import pandas as pd
from matplotlib import pyplot as plt
import re
import itertools
from config import Config
import logging
import sys
import pprint

logger = logging.getLogger("feature")

def dformat(d):
    return pprint.pformat(d, width=1)


        

class Feature(object):

    @staticmethod
    def from_series(series):
        return Feature(series.name, series.values)

    @staticmethod
    def merge_instances(*inst):
        st = Config()
        for f in inst:
            st.update(f.st)
        return Feature(inst[-1].name, inst[-1].data, st)

    def __init__(self, name, data, st=None):
        assert len(data.shape) == 1, "Data should be one dimensional"
        self.name = name
        self.data = data
        self.st = st if not st is None else Config()

    def __getattr__(self, n):
        if n in self.__dict__: return self.__dict__[n]
        if n in self.st: return self.st[n]
        raise KeyError("Failed to find attribute: `{}`".format(n))

    def __hash__(self):
        return hash((self.name, self.data.shape))

    def __eq__(self, other):
        return (
            self.name == other.name and 
            np.all(self.data == other.data)
        )

    @property
    def data_size(self):
        return self.data.shape[0]

    def __repr__(self):
        return "Feature(name={}, data_size={}, stat=\n{})".format(
            self.name, 
            self.data.shape[0],
            "\n".join(["\t{}".format(ss) for ss in str(self.st).split("\n")])
        )

    def plot(self, split_by=None, hist_bins="auto"):
        fig = plt.figure(figsize=(7, 7))
        ax = fig.add_subplot(111)
        assert len(self.data.shape) == 1, "density plots are only relevant for 1-d feature"

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
                    bins=hist_bins, 
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
                    bins=hist_bins, 
                    normed=True
                )
                ax.set_title(
                    "Histogram for a feature `{}`".format(self.name)
                )
                ax.set_xlabel(self.name)
                ax.set_ylabel("Normalized `{}`".format(self.name))
        fig.show()


