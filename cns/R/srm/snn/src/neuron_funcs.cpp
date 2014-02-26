
#include <RcppArmadillo.h>
using namespace Rcpp;

#define LINEAR 1
#define EXP 2

#define PROB_FUNC LINEAR

double probf(const double &u, const List &c) {
#if PROB_FUNC == LINEAR 
    double p = (as<double>(c["pr"]) + (u - as<double>(c["u_rest"]))*as<double>(c["gain_factor"]))/as<double>(c["sim_dim"]);
    if(p < as<double>(c["pr"])/as<double>(c["sim_dim"])) return as<double>(c["pr"])/as<double>(c["sim_dim"]);
    return p;

#elif PROB_FUNC == EXP
//   (beta/alpha)*(log(1+exp(alpha*(tr-u))) -alpha*(tr-u))  
    return ( as<double>(c["beta"])/as<double>(c["alpha"]) ) * 
                ( log( 1 + exp( as<double>(c["alpha"])*(as<double>(c["tr"])-u))) - 
                                    as<double>(c["alpha"])*(as<double>(c["tr"])-u))/as<double>(c["sim_dim"]) ;
#endif    
}

double pstroke(const double &u, const List &c) {
#if PROB_FUNC == LINEAR 
    return as<double>(c["gain_factor"])/as<double>(c["sim_dim"]);
#elif PROB_FUNC == EXP
    return as<double>(c["beta"])/(1+exp(as<double>(c["alpha"])*(as<double>(c["tr"])-u)))/as<double>(c["sim_dim"]);
#endif
}


arma::vec C_calc(bool Yspike, double p, double u, arma::vec  epsps, const List &c) {
    const double pstr = pstroke(u, c);
    return ( pstr/p ) * ( Yspike - p ) * epsps;
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
