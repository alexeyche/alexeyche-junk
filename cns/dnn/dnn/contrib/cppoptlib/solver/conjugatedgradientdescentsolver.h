// CppNumericalSolver
#ifndef CONJUGATEDGRADIENTDESCENTSOLVER_H_
#define CONJUGATEDGRADIENTDESCENTSOLVER_H_

#include "isolver.h"
#include "../linesearch/armijo.h"

namespace cppoptlib {

template<typename T>
class ConjugatedGradientDescentSolver : public ISolver<T, 1> {

 public:
  /**
   * @brief minimize
   * @details [long description]
   *
   * @param objFunc [description]
   */
  void minimize(Problem<T> &objFunc, Vector<T> & x0) {

    size_t iter = 0;
    T gradNorm = 0;

    Vector<T> grad(x0.n_rows);
    Vector<T> grad_old(x0.n_rows);
    Vector<T> Si(x0.n_rows);
    Vector<T> Si_old(x0.n_rows);

    do {
      objFunc.gradient(x0, grad);

      if (iter == 0) {
        Si = -grad;
      } else {
        const double beta = arma::dot(grad, grad) / (arma::dot(grad_old, grad_old));
        Si = -grad + beta * Si_old;
      }

      const double rate = Armijo<T, decltype(objFunc), 1>::linesearch(x0, Si, objFunc) ;

      x0 = x0 + rate * Si;

      grad_old = grad;
      Si_old = Si;

      gradNorm = arma::norm(grad, "inf");
      // std::cout << "iter: "<<iter<< " f = " <<  objFunc.value(x0) << " ||g||_inf "<<gradNorm   << std::endl;
      iter++;

    } while ((gradNorm > this->settings_.gradTol) && (iter < this->settings_.maxIter));

  }

};

} /* namespace cppoptlib */

#endif /* CONJUGATEDGRADIENTDESCENTSOLVER_H_ */
