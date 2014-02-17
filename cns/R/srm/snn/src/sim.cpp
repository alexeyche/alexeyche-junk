
#include <RcppArmadillo.h>

using namespace Rcpp;
using namespace std;

class SRMLayer;
RCPP_EXPOSED_CLASS(SRMLayer)

double asD(const char *name, const List &c) {
    return as<double>(c[name]);
}


typedef vector< vector<unsigned int> > TVecIDs;
typedef vector< vector<double> > TVecNums;

class SRMLayer {
public:
    SRMLayer() {}
    SRMLayer(int N_) : N(N_), ids(N), U(N, arma::fill::zeros), C(N, arma::fill::zeros), W(N), id_conns(N), syn(N) { 
    }
    void prepare(const List &c) {
        U.fill( asD("u_rest", c) );
        syn.fill(0.0);
        C.fill(0.0);
    }

    void simdt(const List &sim_options, const List &constants, List &net)  {
        prepare(constants);
        
        const double T0 = as<double>(sim_options["T0"]);
        const double Tmax = as<double>(sim_options["Tmax"]);
        const double dt = as<double>(sim_options["dt"]);    
        const bool saveStat = as<bool>(sim_options["saveStat"]);    
        const bool learn = as<bool>(sim_options["learn"]);    
        
        arma::vec T = arma::linspace(T0, Tmax, (Tmax-T0)/dt);
        for(size_t ti=0; ti<T.n_elem; ti++) {
        
        }
    }
    
    int N;
    arma::uvec ids;
    arma::vec U;
    arma::vec C;
    TVecNums W;
    TVecIDs id_conns;
    TVecNums syn;
};

class SIM {
public:
    SIM() {}
    void addLayer(SRMLayer l) {
        layers.push_back(l);
    }
    vector<SRMLayer> layers;
};


RCPP_MODULE(snnMod){
    class_<SRMLayer>( "SRMLayer" )
    .constructor<size_t>()
    .field("N", &SRMLayer::N, "Number of neurons")
    .field("ids", &SRMLayer::ids, "Unique ids of neurons")
    .field("U", &SRMLayer::U, "Potentials")
    .field("W", &SRMLayer::W, "Weights")
    .field("id_conns", &SRMLayer::id_conns, "IDs of connections")
    ;
    class_<SIM>( "SIM" )
    .constructor()
    .field("layers", &SIM::layers, "Layers")
    ;
}

