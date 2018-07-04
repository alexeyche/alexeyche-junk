

from sklearn.tree import DecisionTreeClassifier
from sklearn import metrics

from sklearn.preprocessing import StandardScaler
from sklearn.datasets import make_moons, make_circles, make_classification
from sklearn.neural_network import MLPClassifier


from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC
from sklearn.gaussian_process import GaussianProcessClassifier
from sklearn.gaussian_process.kernels import RBF
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.discriminant_analysis import QuadraticDiscriminantAnalysis
from sklearn.neural_network import MLPClassifier

from poc.datasets import *



import numpy as np

names = [
    "Nearest Neighbors", 
    "Linear SVM", 
    "RBF SVM", 
    "Gaussian Process",
    "Decision Tree", 
    "Random Forest", 
    "Neural net",
    "AdaBoost",
    "Naive Bayes", 
    "QDA",
    "MLPClassifier"
]

classifiers = [
    KNeighborsClassifier(3),
    SVC(kernel="linear", C=0.025),
    SVC(gamma=2, C=1),
    GaussianProcessClassifier(1.0 * RBF(1.0), warm_start=True),
    DecisionTreeClassifier(max_depth=5),
    RandomForestClassifier(max_depth=5, n_estimators=10, max_features=1),
    MLPClassifier(max_iter=1000),
    AdaBoostClassifier(),
    GaussianNB(),
    QuadraticDiscriminantAnalysis(),
    MLPClassifier(
        solver="adam",
        activation="relu",
        hidden_layer_sizes=(100,),
        alpha=0.0,
        learning_rate="constant",
        learning_rate_init=0.001,
        max_iter=1000
    )
]

np.random.seed(11)
ds = XorDataset()

# model = DecisionTreeClassifier()
for model_name, model in zip(names, classifiers):
    x_train, y_train = ds.next_train_batch()
    
    y_train = np.argmax(y_train, axis=1)

    x_test, y_test = ds.next_test_batch()
    y_test = np.argmax(y_test, axis=1)

    model.fit(x_train, y_train)


    predicted = model.predict(x_test)

    print('%s, predicting, classification error=%f' % (
        model_name,
        sum( int(predicted[i]) != y_test[i] for i in range(len(y_test))) / float(len(y_test)) 
    ))

# >>> np.mean(ds._x_v)
# 0.026144806425465482
# >>> 