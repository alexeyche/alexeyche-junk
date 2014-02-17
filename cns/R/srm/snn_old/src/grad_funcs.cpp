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

arma::vec Cintegrand(const arma::vec &t, void *data) {
    SInput si = *(SInput*)data; 
}

NumericVector C_calc(bool Yspike, double p, NumericVector epsps, const List &constants) {
    const double gain_factor = as<double>(constants["gain_factor"]);
    const double dt = as<double>(constants["dt"]);
//    static bool pr2 = false;
//    if(!pr2) std::cout << "gain_factor: " << gain_factor << "\n";
//    pr2=true; 
    return ( gain_factor/p ) * ( Yspike - p ) * epsps / 1000;
}


double B_calc(bool Yspike, double p, double pmean, const List &constants) {
    const double gamma = as<const double>(constants["target_rate_factor"]);
    const double targ_rate = as<double>(constants["target_rate"])/1000;
//    static bool pr = false;
//    if(!pr) std::cout << "gamma: " << gamma << " " << " targ_rate " << targ_rate << "\n";
//    pr=true; 
    return ((Yspike*log(p/pmean) - (p - pmean)) - gamma * ( Yspike * log( pmean/targ_rate) - (pmean - targ_rate) ));
}

double ratecalc(const double &weight, const List &constants) {
    return 0.04 * pow(weight, 4)/( pow(weight, 4) + pow(as<double>(constants["ws"]), 4));
}

