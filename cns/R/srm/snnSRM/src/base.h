#ifndef BASE_H
#define BASE_H

#include <Rcpp.h>
using namespace Rcpp;

struct SInput {
  SInput(const List &cc, const IntegerVector &idc, const IntegerVector &id_connc, const NumericVector &wc, const List &netc):
        c(cc), id(idc), id_conn(id_connc), w(wc), net(netc) {}
  const double& get_c(const char *name) const  { 
    return as<double>(c[name]);
  }
  const List &c;
  const IntegerVector &id;
  const IntegerVector &id_conn;
  const NumericVector &w;
  const List &net;
};

struct SSynInput {
  SSynInput(const int syn_idc, const SInput &sic) : syn_id(syn_idc), si(sic) {}
  const int syn_id;
  const SInput &si;
};

#endif
