#ifndef SIM_H
#define SIM_H

#include <RcppArmadillo.h>

using namespace Rcpp;
using namespace std;


typedef vector< arma::uvec > TVecIDs;
typedef vector< arma::vec > TVecNums;
typedef vector< vector<double> > TStatAcc;
typedef vector< vector<arma::vec> > TVecStatAcc;

class SRMLayer;
class SIM;

RCPP_EXPOSED_CLASS(SRMLayer)
RCPP_EXPOSED_CLASS(SIM)

double asD(const char *name, const List &c);

#endif
