// CppNumericalSolver
#include <iostream>

#include "isolver.h"
#include "../linesearch/morethuente.h"

#ifndef LBFGSSOLVER_H_
#define LBFGSSOLVER_H_

namespace cppoptlib {

template<typename T>
class LbfgsSolver : public ISolver<T, 1> {
  public:
    void minimize(Problem<T> &objFunc, Vector<T> & x0) {

        const size_t m = 10;
        const size_t DIM = x0.n_rows;

        Matrix<T> sVector = Zero<T>(DIM, m);
        Matrix<T> yVector = Zero<T>(DIM, m);

        Vector<T> alpha = Zero<T>(m);
        Vector<T> grad(DIM), q(DIM), grad_old(DIM), s(DIM), y(DIM);
        objFunc.gradient(x0, grad);
        Vector<T> x_old = x0;
        Vector<T> x_old2 = x0;

        size_t iter = 0, j = 0, globIter = 0;
        double H0k = 1;

        T gradNorm = 0;

        do {

            const T relativeEpsilon = static_cast<T>(0.0001) * std::max(static_cast<T>(1.0), arma::norm(x0));

            if (arma::norm(grad) < relativeEpsilon)
                break;

            //Algorithm 7.4 (L-BFGS two-loop recursion)
            q = grad;
            const int k = std::min(m, iter);

            // for i = k − 1, k − 2, . . . , k − m
            for (int i = k - 1; i >= 0; i--) {
                // alpha_i <- rho_i*s_i^T*q
                const double rho = 1.0 / arma::dot(static_cast<Vector<T>>(sVector.col(i)), static_cast<Vector<T>>(yVector.col(i)));
                
                alpha(i) = rho * arma::dot(static_cast<Vector<T>>(sVector.col(i)), q);
                // q <- q - alpha_i*y_i
                q = q - alpha(i) * yVector.col(i);
            }
            // r <- H_k^0*q
            q = H0k * q;
            //for i k − m, k − m + 1, . . . , k − 1
            for (int i = 0; i < k; i++) {
                // beta <- rho_i * y_i^T * r
                const double rho = 1.0 / arma::dot(static_cast<Vector<T>>(sVector.col(i)), static_cast<Vector<T>>(yVector.col(i)));
                const double beta = rho * arma::dot(static_cast<Vector<T>>(yVector.col(i)), q);
                // r <- r + s_i * ( alpha_i - beta)
                q = q + sVector.col(i) * (alpha(i) - beta);
            }
            // stop with result "H_k*f_f'=q"

            // any issues with the descent direction ?
            double descent = arma::dot(-grad, q);
            double alpha_init =  1.0 / arma::norm(grad);
            if (descent > -0.0001 * relativeEpsilon) {
                q = -1 * grad;
                iter = 0;
                alpha_init = 1.0;
            }

            // find steplength
            const double rate = MoreThuente<T, decltype(objFunc), 1>::linesearch(x0, -q,  objFunc, alpha_init) ;
            // update guess
            x0 = x0 - rate * q;

            grad_old = grad;
            objFunc.gradient(x0, grad);

            s = x0 - x_old;
            y = grad - grad_old;

            // update the history
            if (iter < m) {
                sVector.col(iter) = s;
                yVector.col(iter) = y;
            } else {

                sVector.head_cols(m - 1) = sVector.tail_cols(m - 1).eval();
                sVector.tail_cols(1) = s;
                yVector.head_cols(m - 1) = yVector.tail_cols(m - 1).eval();
                yVector.tail_cols(1) = y;
            }
            // update the scaling factor
            H0k = arma::dot(y, s) / static_cast<double>(arma::dot(y, y));

            x_old = x0;
            gradNorm = arma::norm(grad, "inf");
            // std::cout << "iter: "<<globIter<< ", f = " <<  objFunc.value(x0) << ", ||g||_inf "
            // <<gradNorm  << std::endl;

            iter++;
            globIter++;
            j++;

        } while ((gradNorm > this->settings_.gradTol) && (j < this->settings_.maxIter));

    }

};

}
/* namespace cppoptlib */

#endif /* LBFGSSOLVER_H_ */
