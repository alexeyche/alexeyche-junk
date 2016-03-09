// CppNumericalSolver
#include <iostream>

#include "isolver.h"
#include "../linesearch/armijo.h"

#ifndef NEWTONDESCENTSOLVER_H_
#define NEWTONDESCENTSOLVER_H_

namespace cppoptlib {

template<typename T>
class NewtonDescentSolver : public ISolver<T, 2> {
  public:
    void minimize(Problem<T> &objFunc, Vector<T> & x0) {

        const size_t DIM = x0.n_rows;

        Vector<T> grad = Zero<T>(DIM);
        Matrix<T> hessian = Zero<T>(DIM, DIM);

        T gradNorm = 0;

        size_t iter = 0;
        do {
            objFunc.gradient(x0, grad);
            objFunc.hessian(x0, hessian);
            hessian += (1e-5) * Identity<T>(DIM, DIM);
            Vector<T> delta_x = hessian.i() * (-grad);
            // Vector<T> delta_x = hessian.lu().solve(-grad);

            const double rate = Armijo<T, decltype(objFunc), 1>::linesearch(x0, delta_x, objFunc) ;
            x0 = x0 + rate * delta_x;
            gradNorm = arma::norm(grad, "inf");
            // std::cout << "iter: "<<iter<< ", f = " <<  objFunc.value(x0) << ", ||g||_inf "<<gradNorm  << std::endl;
            iter++;

        } while ((gradNorm > this->settings_.gradTol) && (iter < this->settings_.maxIter));
    }
};

}
/* namespace cppoptlib */

#endif /* NEWTONDESCENTSOLVER_H_ */
