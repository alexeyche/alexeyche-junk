
from util import *

import numpy as np

from sklearn.neighbors import NearestNeighbors as sknn

from metrics import knn

faiss = get_faiss()


def running_ivf_flat(X_train, X_test, metric, expected_dist):
    nn = expected_dist.shape[1]

    print("Running IndexFlatL2/IndexIVFFlat")
    for K in [32, 64]:
        quantizer = faiss.IndexFlatL2(X_train.shape[1])
        index = faiss.IndexIVFFlat(quantizer, X_train.shape[1], K, faiss.METRIC_L2)

        with Timing("Building index"):
            index.train(X_train)
            index.add(X_train)

        for probe in [5, 10, 20, 40]:
            index.nprobe = probe

            with Timing("Querying"):
                _, idx = index.search(X_test, nn)

            with Timing("Evaluating"):
                actual_dist = np.asarray([
                    [metric(X_test[i], vv) for vv in X_train[idx[i]]]
                    for i in range(len(idx))
                ])


            recall = knn(expected_dist, actual_dist, nn)
            print("K = {}, probe = {}, recall = {:.4f}".format(K, probe, recall))


def running_knn_bruteforce(X_train, X_test, metric, expected_dist):
    nn = expected_dist.shape[1]

    with Timing("Building index `Brute`"):
        sknn_model = sknn(algorithm="brute", metric=metric.__name__)
        sknn_model.fit(X_train)

    with Timing("Querying `Brute`"):
        idx = sknn_model.kneighbors(X_test, return_distance=False, n_neighbors=nn)

    with Timing("Evaluating `Brute`"):
        actual_dist = np.asarray([
            [metric(X_test[i], vv) for vv in X_train[idx[i]]]
            for i in range(len(idx))
        ])

    recall = knn(expected_dist, actual_dist, nn)
    print("Recall: {:.4f}".format(recall))  # 0.99
