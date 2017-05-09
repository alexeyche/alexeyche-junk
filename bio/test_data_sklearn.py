
from sklearn.tree import DecisionTreeClassifier
from sklearn import metrics

from sklearn.preprocessing import StandardScaler
from sklearn.datasets import make_moons, make_circles, make_classification
# from sklearn.neural_network import MLPClassifier
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC
# from sklearn.gaussian_process import GaussianProcessClassifier
# from sklearn.gaussian_process.kernels import RBF
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.discriminant_analysis import QuadraticDiscriminantAnalysis

from datasets import get_toy_data


names = [
    "Nearest Neighbors", 
    "Linear SVM", 
    "RBF SVM", 
    # "Gaussian Process",
    "Decision Tree", 
    "Random Forest", 
    # "Neural net",
    "AdaBoost",
    "Naive Bayes", 
    "QDA"
]

classifiers = [
    KNeighborsClassifier(3),
    SVC(kernel="linear", C=0.025),
    SVC(gamma=2, C=1),
    # GaussianProcessClassifier(1.0 * RBF(1.0), warm_start=True),
    DecisionTreeClassifier(max_depth=5),
    RandomForestClassifier(max_depth=5, n_estimators=10, max_features=1),
    # MLPClassifier(alpha=1),
    AdaBoostClassifier(),
    GaussianNB(),
    QuadraticDiscriminantAnalysis()
]

# model = DecisionTreeClassifier()
for model_name, model in zip(names, classifiers):
    x_values, y_values = get_toy_data(seed=2)


    n_train = (4 * x_values.shape[0]/5)
    n_valid = (1 * x_values.shape[0]/5)
            
    x_train = x_values[:n_train]
    x_test = x_values[n_train:(n_train+n_valid)]

    y_train = y_values[:n_train]
    y_test = y_values[n_train:(n_train+n_valid)]


    model.fit(x_train, y_train)


    predicted = model.predict(x_test)

    print '%s, predicting, classification error=%f' % (
        model_name,
        sum( int(predicted[i]) != y_test[i] for i in range(len(y_test))) / float(len(y_test)) 
    )

