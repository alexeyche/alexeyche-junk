
#include <RcppArmadillo.h>

using namespace Rcpp;
using namespace std;

class SRMLayer;
class SIM;
RCPP_EXPOSED_CLASS(SRMLayer)
RCPP_EXPOSED_CLASS(SIM)

double asD(const char *name, const List &c) {
    return as<double>(c[name]);
}


typedef vector< arma::uvec > TVecIDs;
typedef vector< arma::vec > TVecNums;

class SRMLayer {
public:
    SRMLayer() {}
    SRMLayer(int N_) : N(N_), ids(N), U(N, arma::fill::zeros), C(N, arma::fill::zeros), W(N), id_conns(N), syn(N) { 
    }
    void prepare(const List c) {
        U.fill( asD("u_rest", c) );
        for(size_t syn_i=0; syn_i < syn.size(); syn_i++) {
            syn[syn_i].fill(0.0);
        }
        C.fill(0.0);
    }

    void simdt(const double &dt, const List &constants, List &net)  {
        prepare(constants);
        
       
        
    }
    
    // consts 
    int N;
    arma::uvec ids;
    TVecNums W;
    TVecIDs id_conns;
    
    // vars
    TVecNums syn;
    arma::vec U;
    arma::vec C;
};

//class SIM {
//public:
//    SIM() {}
//    void addLayer(SRMLayer l) {
//        layers.push_back(l);
//    }
//    vector<SRMLayer> layers;
//};
//
//namespace Rcpp {
//template <> SEXP wrap<SRMLayer>( const SRMLayer& el ) {
//        Rcpp::Language call( "new", Symbol( "SRMLayer" )) ;
//        return call.eval();
//};
//
//template <> SRMLayer as<SRMLayer>( SEXP s ) throw(not_compatible) {
//        try {
//            if ( TYPEOF(s) != S4SXP ) {
//                ::Rf_error( "supplied object is not of type TestClass." );
//            }
//
//            Rcpp::S4 s4obj( s );
//            if ( !s4obj.is("Rcpp_TestClass" ) ) {
//                ::Rf_error( "supplied object is not of type TestClass." );
//            }
//
//            Rcpp::Environment env( s4obj );
//            Rcpp::XPtr<SRMLayer> xptr( env.get(".pointer") );
//
//            // build new TestClass object with copied data
//            return SRMLayer();
//        }
//        catch(...) {
//            ::Rf_error( "supplied object could not be converted to TestClass." );
//        }
//};
//};

RCPP_MODULE(snnMod){
    class_<SRMLayer>( "SRMLayer" )
    .constructor()
    .constructor<size_t>()
    .field("N", &SRMLayer::N, "Number of neurons")
    .field("ids", &SRMLayer::ids, "Unique ids of neurons")
    .field("U", &SRMLayer::U, "Potentials")
    .field("W", &SRMLayer::W, "Weights")
    .field("id_conns", &SRMLayer::id_conns, "IDs of connections")
    .field("syn", &SRMLayer::syn, "synapses")
    .method("prepare", &SRMLayer::prepare)
    ;
//    class_<SIM>( "SIM" )
//    .constructor()
//    .field("layers", &SIM::layers, "Layers")
//    ;
}

