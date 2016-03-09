// CppNumericalSolver
#ifndef META_H
#define META_H

#include <iostream>

#define ARMA_DONT_USE_WRAPPER
#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS

#include <armadillo>



namespace cppoptlib {

template <typename T>
using Vector = arma::Col<T>;

template <typename T>
using Matrix = arma::Mat<T>;

template <typename T>
Vector<T> Random(size_t v) {
    Vector<T> vv(v, arma::fill::randu);
    return -1.0 + vv*2.0;
}

template <typename T>
Matrix<T> Random(size_t v, size_t c) {
    Matrix<T> vv(v, c, arma::fill::randu);
    return -1.0 + vv*2.0;
}

template <typename T>
Matrix<T> Identity(size_t v, size_t c) {
    Matrix<T> vv(v, c, arma::fill::eye);
    return vv;
}
template <typename T>
Matrix<T> Zero(size_t v, size_t c) {
    Matrix<T> vv(v, c, arma::fill::zeros);
    return vv;
}

template <typename T>
Vector<T> Zero(size_t v) {
    Vector<T> vv(v, arma::fill::zeros);
    return vv;
}


template <typename T>
Vector<T> Ones(size_t v) {
    Vector<T> vv(v, arma::fill::ones);
    return vv;
}

template <typename T>
Vector<T> ElWiseMax(const Vector<T> &v, const Vector<T>& mv) {
    Vector<T> newv(v);
    for (size_t i=0; i<v.size(); ++i) {
        newv(i) = std::max(v(i), mv(i));
    }
    return newv;
}


template <typename T>
Vector<T> ElWiseMin(const Vector<T> &v, const Vector<T>& mv) {
    Vector<T> newv(v);
    for (size_t i=0; i<v.size(); ++i) {
        newv(i) = std::min(v(i), mv(i));
    }
    return newv;
}

template <typename T>
double AsScalar(const Vector<T> &m) {
    return m(0);
}

typedef struct Options {
    double gradTol;
    double rate;
    size_t maxIter;
    size_t m;

    Options() {
        rate = 0.00005;
        maxIter = 100000;
        gradTol = 1e-4;
        m = 10;

    }
} Options;

template<typename T>
bool checkConvergence(T val_new, T val_old, Vector<T> grad, Vector<T> x_new, Vector<T> x_old) {

    T ftol = 1e-10;
    T gtol = 1e-8;
    T xtol = 1e-32;

    // value crit.
    if((x_new-x_old).cwiseAbs().maxCoeff() < xtol)
        return true;

    // // absol. crit
    if(abs(val_new - val_old) / (abs(val_new) + ftol) < ftol) {
        std::cout << abs(val_new - val_old) / (abs(val_new) + ftol) << std::endl;
        std::cout << val_new << std::endl;
        std::cout << val_old << std::endl;
        std::cout << abs(val_new - val_old) / (abs(val_new) + ftol) << std::endl;
        return true;
    }

    // gradient crit
    T g = arma::norm(grad, "inf");
    if (g < gtol)
        return true;
    return false;
}

}
#endif /* META_H */
