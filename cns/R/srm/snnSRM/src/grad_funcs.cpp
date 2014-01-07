#include "grad_funcs.h"

double p_stroke(const double &t, const SInput &si) {
 //printf("beta: %f, alpha: %f, tr: %f, u(t,si) = %f\n", (double)si.c["beta"], (double)si.c["alpha"], (double)si.c["tr"], u(t, si));
 return si.get_c("beta")/(1+exp(si.get_c("alpha")*(si.get_c("tr")-u(t, si))));
}

double grab_epsp(const double &t, const SSynInput &ssyn) {
  double e_syn = 0;
  const NumericVector &sp(ssyn.si.net[ssyn.syn_id-1]);
  for(int syn_id=sp.size()-1; syn_id>=0; syn_id--) {
    if(t - sp[syn_id] > EPSP_WORK_WINDOW) break;
    e_syn += epsp(t - sp[syn_id], ssyn.si.c);
  }
  return e_syn;
}

