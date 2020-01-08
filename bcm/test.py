import scipy.io
import numpy as np
import tensorflow as tf
from tensorflow.keras import datasets, layers, optimizers, Sequential, metrics, callbacks

mat = scipy.io.loadmat('mnist_all.mat')

def read_mnist(mat, label, num_classes=10):
    X = np.zeros((0, 784))
    Y = np.zeros((0, num_classes))
    for i in range(num_classes):
        data = mat[label + str(i)] # [:500]

        X = np.concatenate((X, data), axis=0)

        Yi = np.zeros((data.shape[0], num_classes))
        Yi[:, i] = 1.0
        Y = np.concatenate((Y, Yi), axis=0)

    X = X / 255.0
    return X.astype(np.float32), Y.astype(np.float32)

def train_mlp(
    X,
    Y,
    Xt,
    Yt,
    hidden_layer_size=None,
    epochs=300,
    init_learning_rate=0.02,
    verbose=0
):
    def read_mnist_tf(X, Y):
        return (
            tf.data.Dataset.from_tensor_slices((X,Y))
                .shuffle(X.shape[0])
                .batch(100)
        )

    def scheduler(epoch):
        return init_learning_rate * (1.0 - epoch / epochs)

    train_dataset = read_mnist_tf(X, Y)
    test_dataset = read_mnist_tf(Xt, Yt)

    if hidden_layer_size is not None:
        model = Sequential([
            layers.Dense(hidden_layer_size, activation='relu'),
            layers.Dense(Y.shape[1])
        ])
    else:
        model = Sequential([
            layers.Dense(Y.shape[1])
        ])

    model.compile(
        optimizer=optimizers.SGD(init_learning_rate),
        loss=tf.losses.CategoricalCrossentropy(from_logits=True),
        metrics=['accuracy']
    )
    model.fit(
        train_dataset, epochs=epochs,
        validation_data=test_dataset,
        validation_steps=2,
        callbacks=[
            callbacks.EarlyStopping(monitor='val_loss', patience=10),
            callbacks.LearningRateScheduler(scheduler)
        ],
        verbose=verbose
    )
    return max(model.history.history["val_accuracy"])


X, Y = read_mnist(mat, "train", 10)
Xt, Yt = read_mnist(mat, "test", 10)

train_mlp(X, Y, Xt, Yt, hidden_layer_size=100, verbose=1)
