

import matplotlib as mpl
import matplotlib.cm as cm
import numpy as np
from matplotlib import pyplot as plt

def plot_importance(values, labels):
    color_map = cm.ScalarMappable(
        norm=mpl.colors.Normalize(
            vmin=np.min(values),
            vmax=np.max(values) + np.max(values) * 0.5 # highest value is too yellow
        ),
        cmap=cm.hot
    )

    indices = np.asarray(
        tuple(reversed(np.argsort(values)[::-1]))
    )

    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    ax.barh(
        np.arange(len(labels)),
        values[indices],
        color=color_map.to_rgba(values[indices])
    )
    ax.set_yticklabels(labels[indices], fontsize="x-small")
    ax.set_yticks(range(len(labels)))
    ax.set_ylim([-1, len(labels)])
    ax.set_title(
        "Feature importances"
    )
    fig.show()


def plot_tree(model, fp):
    # veery dirty, didn't find another way :C
    from sklearn.tree import export_graphviz
    from sklearn import tree
    from graphviz import Source
    import pydot
    from StringIO import StringIO
    import subprocess as sub
    import tempfile

    column_names = fp.train_predictors().names()
    target_names = fp.train_targets().names()
    assert len(target_names) == 1
    target_name = target_names[0]

    new_file, filename = tempfile.mkstemp(suffix=".png")

    dotfile = StringIO()

    tree.export_graphviz(
        model.instance,
        out_file=dotfile,
        rounded=True,
        proportion = False,
        feature_names = column_names,
        precision = 2,
        class_names = [
            "Not {}".format(target_name),
            target_name
        ],
        filled = True
    )
    v = pydot.graph_from_dot_data(dotfile.getvalue())[0]
    v.write_png(filename)
    sub.call(["open", filename])
