

#include <Rcpp.h>
#include <cmath>
#include <stdio.h>

RcppExport SEXP hA2(SEXP vector) {
BEGIN_RCPP
    Rcpp::NumericVector orig(vector);                   // keep a copy (as the classic version does)
    Rcpp::NumericVector vec(orig.size());               // create a target vector of the same size
    
    // we could query size via
    //   int n = vec.size();
    // and loop over the vector, but using the STL is so much nicer
    // so we use a STL transform() algorithm on each element
    std::transform(orig.begin(), orig.end(), vec.begin(), ::sqrt);

    return Rcpp::List::create(Rcpp::Named( "result" ) = vec,
                              Rcpp::Named( "original" ) = orig) ;
END_RCPP
}
