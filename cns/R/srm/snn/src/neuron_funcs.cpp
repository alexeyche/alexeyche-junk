
#include <RcppArmadillo.h>
using namespace Rcpp;

#define LINEAR 1
#define EXP 2

#define PROB_FUNC EXP

double probf(const double &u, const List &c) {
#if PROB_FUNC == LINEAR 
    double p = (as<double>(c["pr"]) + (u - as<double>(c["u_rest"]))*as<double>(c["gain_factor"]))/as<double>(c["sim_dim"]);

#elif PROB_FUNC == EXP
//   (beta/alpha)*(log(1+exp(alpha*(tr-u))) -alpha*(tr-u))  
    double p = (as<double>(c["pr"]) + ( as<double>(c["beta"])/as<double>(c["alpha"]) ) * 
                ( log( 1 + exp( as<double>(c["alpha"])*(as<double>(c["tr"])-u))) - 
                                    as<double>(c["alpha"])*(as<double>(c["tr"])-u)))/as<double>(c["sim_dim"]);
#endif    
    if(p < as<double>(c["pr"])/as<double>(c["sim_dim"])) return as<double>(c["pr"])/as<double>(c["sim_dim"]);
    return p;
}



// [[Rcpp::export]]
double probFun(double u, const List c) {
    return probf(u, c);
}


double pstroke(const double &u, const List &c) {
#if PROB_FUNC == LINEAR 
    return as<double>(c["gain_factor"])/as<double>(c["sim_dim"]);
#elif PROB_FUNC == EXP
    return as<double>(c["beta"])/(1+exp(as<double>(c["alpha"])*(as<double>(c["tr"])-u)))/as<double>(c["sim_dim"]);
#endif
}


arma::vec C_calc(bool Yspike, double p, double u, arma::vec epsps, const List &c) {
    const double pstr = pstroke(u, c);
    return ( pstr/p ) * ( Yspike - p ) * epsps;
}

double B_calc(bool Yspike, double p, double pmean, const List &c) {
    const double gamma = as<const double>(c["target_rate_factor"]);
    const double targ_rate = as<double>(c["target_rate"])/as<double>(c["sim_dim"]);
//    std::cout << "Yspike " << float(Yspike) << " p " << p << " pmean " << pmean << "\n";
    return ((Yspike*log(p/pmean) - (p - pmean)) - gamma * ( Yspike * log( pmean/targ_rate) - (pmean - targ_rate) ));
}

arma::vec ratecalc(const arma::vec &weights, const double &ws4) {
    //arma::vec w4 = arma::pow(weights, 4);
    arma::vec wr(weights.n_elem, arma::fill::zeros);
    for(size_t wi=0; wi<weights.n_elem; wi++) {
        if(weights(wi) > 0.5) {
            wr(wi) = 0.04;
        } else
        if((weights(wi) < 0.5)&&(weights(wi) > 0.1)) {
            wr(wi) = 0.01;
        } else 
        if((weights(wi) < 0.1)&&(weights(wi) > 0.01)) {
            wr(wi) = 0.0001;
        } 
    }
    return wr;
}
arma::vec new_ratecalc(const arma::vec &weights, const List &c) {
    return 0.04 * 1/( 1 + arma::exp(as<double>(c["ws"])-weights));
}
