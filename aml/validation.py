import numpy as np
import logging

from sklearn.metrics import confusion_matrix
from sklearn.metrics import classification_report
from sklearn.utils.multiclass import unique_labels
from metric import *
import pandas as pd
from tabulate import tabulate

from operation import Operation


logger = logging.getLogger("validation")


def pretty_table(header, data, columns, rows):
    return pretty_df(
        header,
        pd.DataFrame(data, index=rows, columns=columns)
    )


def pretty_df(header, df):
    df_s = tabulate(
        df,
        headers='keys',
        tablefmt='psql'
    )

    return (
        "\t" + header + "\n" +
        "\t" + "\t".join(df_s.splitlines(True))
    )


class Validation(Operation):
    def __init__(
        self,
        metrics=(),
    ):
        self.metrics = metrics

    def do(self, mo):
        self.validate(mo.model, mo.test)
        return None


class VClassificationReport(Validation):
    def __init__(self):
        super(VClassificationReport, self).__init__(
            metrics=(
                AUCMetric(),
                F1Metric(),
                AccuracyMetric()
            )
        )

    def validate(self, model, test):
        pred_test_y = model.predict(test.x)
        cr = classification_report(test.y, pred_test_y, output_dict=True)
        cm = confusion_matrix(test.y, pred_test_y)

        mvals = []
        mnames = []
        for metric in self.metrics:
            v = metric(test.y, pred_test_y)
            mvals.append(v)
            mnames.append(metric.name())

        labs = unique_labels(test.y)

        report = "\n"
        report += pretty_df("Report", pd.DataFrame(cr).transpose())
        report += "\n\n"
        report += pretty_table("Metric results", mvals, ["Metric"], mnames)
        report += "\n\n"
        report += pretty_table("Confusion matrix", cm, labs, labs)

        logger.info("VClassificationReport: \n{}".format(report))
