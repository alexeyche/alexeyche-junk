import numpy as np
import logging

import pandas as pd
from tabulate import tabulate
from collections import OrderedDict

from metric import *
from operation import Operation
from feature_pool import FeaturePool
from model import Model, ModelInstance

from sklearn.metrics import confusion_matrix
from sklearn.metrics import classification_report
from sklearn.utils.multiclass import unique_labels




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
        assert len(self.metrics) > 0, "Empty tuple of metrics"

        _, test_x, test_y = mo.fp.test_arrays()
        mo = Model.Output(
            tuple(
                self.validate(m, test_x, test_y)
                for m in mo.models
            ),
            mo.fp
        )

        if len(mo.models) > 1:
            summary = pretty_df(
                "Summary report",
                Validation.get_summary_df(mo, self.metrics[0].name)
            )
            logger.info("VClassificationReport: \n\n{}".format(summary))
        return mo

    @staticmethod
    def get_summary_df(mo, key):
        df = pd.DataFrame(dict([
            (m.name, m.st.validation_metrics)
            for m in mo.models
        ])).transpose()
        return df.sort_values(by=key, ascending=False)



class VClassificationReport(Validation):
    def __init__(self):
        super(VClassificationReport, self).__init__(
            metrics=(
                AUCMetric(),
                F1Metric(),
                AccuracyMetric()
            )
        )

    def validate(self, model, test_x, test_y):
        pred_test_y = model.predict(test_x)
        cr = classification_report(test_y, pred_test_y, output_dict=True)
        cm = confusion_matrix(test_y, pred_test_y)

        validation_metrics = OrderedDict()
        for metric in self.metrics:
            v = metric(test_y, pred_test_y)
            validation_metrics[metric.name] = v

        labs = unique_labels(test_y)

        report = "\n"
        report += "\tClassificationReport for `{}`\n".format(model.name)
        report += "\n"
        report += pretty_df("Report", pd.DataFrame(cr).transpose())
        report += "\n\n"
        report += pretty_table(
            "Metric results",
            validation_metrics.values(),
            ["Metric"],
            validation_metrics.keys()
        )
        report += "\n\n"
        report += pretty_table("Confusion matrix", cm, labs, labs)

        logger.info("VClassificationReport: \n{}".format(report))


        return ModelInstance.apply_config(
            model,
            validation_metrics=validation_metrics
        )
