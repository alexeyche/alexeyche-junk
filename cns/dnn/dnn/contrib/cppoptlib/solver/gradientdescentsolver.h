// CppNumericalSolver
#ifndef GRADIENTDESCENTSOLVER_H_
#define GRADIENTDESCENTSOLVER_H_

#include "isolver.h"
#include "../linesearch/morethuente.h"

namespace cppoptlib {

template<typename T>
class GradientDescentSolver : public ISolver<T, 1> {

 public:
  /**
   * @brief minimize
   * @details [long description]
   *
   * @param objFunc [description]
   */
  void minimize(Problem<T> &objFunc, Vector<T> & x0) {

    Vector<T> direction(x0.n_rows);
    size_t iter = 0;
    T gradNorm = 0;
    do {
      objFunc.gradient(x0, direction);
      const T rate = MoreThuente<T, decltype(objFunc), 1>::linesearch(x0, -direction, objFunc) ;
      x0 = x0 - rate * direction;
      gradNorm = arma::norm(direction, "inf");
      // std::cout << "iter: "<<iter<< " f = " <<  objFunc.value(x0) << " ||g||_inf "<<gradNorm  << std::endl;
      iter++;
    } while ((gradNorm > this->settings_.gradTol) && (iter < this->settings_.maxIter));

  }

};

} /* namespace cppoptlib */

#endif /* GRADIENTDESCENTSOLVER_H_ */
