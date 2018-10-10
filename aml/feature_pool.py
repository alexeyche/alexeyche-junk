
from config import Config
from transform import TClean
import numpy as np
import pandas as pd
import logging
from feature import Feature
from sklearn.decomposition import PCA

from matplotlib import pyplot as plt


logger = logging.getLogger("feature_pool")

class FeaturePool(object):
    @staticmethod
    def from_dataframe(df):
        return FeaturePool([Feature.from_series(s) for _, s in df.iteritems()])

    @staticmethod
    def to_dataframe(fp):
        return pd.DataFrame(np.asarray([f.data for f in fp]).T, columns=[f.name for f in fp])

    @staticmethod
    def to_array(fp):
        return np.asarray([f.data for f in fp]).T

    def __init__(self, features):
        self.features = features

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
        assert plot or split_by is None, \
            "`split_by` option doesn't make sense without `plot` == True"
        pca = PCA(n_components = 2)
        
        if split_by is not None:
            X = FeaturePool.to_array(FeaturePool([f for f in self.features if f.name != split_by]))
        else:
            X = FeaturePool.to_array(self)

        pc = pca.fit_transform(X)
        if plot:
            fig = plt.figure(figsize=(7, 7))
            ax = fig.add_subplot(111)
            if split_by is not None:
                ax.scatter(pc[:, 0], pc[:, 1], c=self[split_by].data)
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
        corrmat = FeaturePool.to_dataframe(self).corr()

        if plot:
            import seaborn as sns

            f = plt.figure(figsize=(7, 7))
            g = sns.heatmap(corrmat, vmax=.8, square=True)
            g.set_yticklabels(g.get_yticklabels(), rotation = 'horizontal', fontsize = 8)
            g.set_xticklabels(g.get_xticklabels(), rotation = 'vertical', fontsize = 8)
            f.show()

        return corrmat
