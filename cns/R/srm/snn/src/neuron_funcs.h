#ifndef NEURON_FUCNS
#define NEURON_FUCNS

double g(const double &u, const List &c);
arma::vec C_calc(bool Yspike, double p, arma::vec  epsps, const List &constants);
double B_calc(bool Yspike, double p, double pmean, const List &constants);
arma::vec ratecalc(const arma::vec &weights, const List &constants);

#endif
