#include <Rcpp.h>
using namespace Rcpp;

double epsp(const double &s, const double &e0, const double &tm, const double &ts) {
    if(s<0) return 0;
    return e0*(exp(-s/tm)-exp(-s/ts));
}
double nu(const double &s, const double &u_abs, const double &u_r, const double &trf, const double &trs, const double &dr) {
    if(s<0) return 0;
    if(s<dr) return u_abs;
    return u_abs*exp(-(s+dr)/trf) + u_r*exp(-s/trs);
}
double u(const double &t, std::vector<double> y) {
}

// [[Rcpp::export]]
SEXP integrate(SEXP constants, SEXP int_options, SEXP neuron_id, SEXP neuron_id_conn, SEXP neuron_w, SEXP net) {
    RcppParams par_const(constants);
    double e0 = par_const.getDoubleValue("e0");
    double ts = par_const.getDoubleValue("ts");
    double tm = par_const.getDoubleValue("tm");
    double u_abs = par_const.getDoubleValue("u_abs");
    double u_r = par_const.getDoubleValue("u_r");
    double trf = par_const.getDoubleValue("trf");
    double trs = par_const.getDoubleValue("trs");
    double dr = par_const.getDoubleValue("dr");
    double alpha = par_const.getDoubleValue("alpha");
    double beta = par_const.getDoubleValue("beta");
    double tr = par_const.getDoubleValue("tr");
    RcppParams par_int(int_options);
    double T0 = par_int.getDoubleValue("T0");
    double Tmax = par_int.getDoubleValue("Tmax");
    RcppParams par_neuron(neuron);
    RcppVector<int> y_id(neuron_id);
    RcppVector<int> id_conn(neuron_id_conn);
    RcppVector<double> w(neuron_w);

}    
