#include <Rcpp.h>
using namespace Rcpp;

// Below is a simple example of exporting a C++ function to R. You can
// source this function into an R session using the Rcpp::sourceCpp 
// function (or via the Source button on the editor toolbar)

// For more on using Rcpp click the Help button on the editor toolbar

// [[Rcpp::export]]
SEXP timesTwo(NumericVector x) {
   NumericVector v(2);
   v[0] = x[0]*2;
   v[1] = x[0]*2;
   return v;
}
