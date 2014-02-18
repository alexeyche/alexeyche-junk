
#include <RcppArmadillo.h>
using namespace Rcpp;


double g(const double &u, const List &c) {
    return (as<double>(c["pr"]) + (u - as<double>(c["u_rest"]))*as<double>(c["gain_factor"]))/as<double>(c["sim_dim"]);
}

arma::vec C_calc(bool Yspike, double p, arma::vec  epsps, const List &c) {
    const double gain_factor = as<double>(c["gain_factor"]);
    const double dt = as<double>(c["dt"]);
    const double sim_dim = as<double>(c["sim_dim"]);
    return ( gain_factor/p ) * ( Yspike - p ) * epsps / sim_dim;
}

double B_calc(bool Yspike, double p, double pmean, const List &c) {
    const double gamma = as<const double>(c["target_rate_factor"]);
    const double targ_rate = as<double>(c["target_rate"])/as<double>(c["sim_dim"]);
//    std::cout << "Yspike " << float(Yspike) << " p " << p << " pmean " << pmean << "\n";
    return ((Yspike*log(p/pmean) - (p - pmean)) - gamma * ( Yspike * log( pmean/targ_rate) - (pmean - targ_rate) ));
}

arma::vec ratecalc(const arma::vec &weights, const List &c) {
    return 0.04 * arma::pow(weights, 4)/( arma::pow(weights, 4) + pow(as<double>(c["ws"]), 4));
}
