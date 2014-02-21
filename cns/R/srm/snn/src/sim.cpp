
#include <RcppArmadillo.h>

using namespace Rcpp;
using namespace std;

#include "neuron_funcs.h"

class SRMLayer;
class SIM;
RCPP_EXPOSED_CLASS(SRMLayer)
RCPP_EXPOSED_CLASS(SIM)

double asD(const char *name, const List &c) {
    return as<double>(c[name]);
}

class NetSim {
public:    
    NetSim(List &net_) : net(net_), active_ids(net.size(), arma::fill::zeros) {}
    int getNumSpikes(size_t i, const double &t, const double &dt) {
        size_t c_id = i-1;
        
        int num_sp = 0;
        NumericVector sp = as<NumericVector>(net[c_id]);
        for(size_t sp_i = active_ids(c_id); sp_i < sp.size(); sp_i++) {
            if (sp[sp_i] >= t-dt) {
                if(sp[sp_i] < t) {
                    // spike in (t-dt, t]
                    num_sp += 1;
                } else {
                    // this is spike in future;
                    break;
                }
            } else {
                active_ids(c_id)+=1; // this is spike in past
            }
        }
        return num_sp;
    }
    void push_back(size_t id, const double &t) {
        size_t c_id = id-1;
        NumericVector sp = net[c_id];
        sp.push_back(t);
        net[c_id] = sp;
    }
    List net;
    arma::ivec active_ids;
};


typedef vector< arma::uvec > TVecIDs;
typedef vector< arma::vec > TVecNums;
typedef vector< vector<double> > TStatAcc;
typedef vector< vector<arma::vec> > TVecStatAcc;

class SRMLayer {
public:
    SRMLayer(int &N_, arma::uvec &ids_, TVecNums &W_, TVecIDs &id_conns_, TVecNums &syn_, TVecNums &syn_spec_, arma::vec &a_, TVecNums &C_, arma::vec &pacc_, int incr_) : 
                        N(N_),
                        ids(ids_),
                        W(W_),
                        id_conns(id_conns_),
                        syn(syn_),
                        syn_spec(syn_spec_),
                        a(a_),
                        C(C_),
                        pacc(pacc_),
                        incr(incr_)
    {}
    SRMLayer(const SRMLayer &l) : N(l.N), ids(l.ids), W(l.W), id_conns(l.id_conns), syn(l.syn), syn_spec(l.syn_spec), a(l.a), C(l.C), pacc(l.pacc), incr(l.incr) {}
    SRMLayer(int N_) : N(N_), ids(N), a(N, arma::fill::zeros), C(N), W(N), id_conns(N), syn(N), syn_spec(N), pacc(N, arma::fill::zeros), incr(0) { }

    void prepare(const List &c) {
        stat_p.clear(); stat_u.clear(); stat_B.clear(); stat_C.clear(); stat_W.clear();
        for(size_t ni=0; ni < N; ni++) {
            syn[ni].fill(0.0);
            C[ni].fill(0.0);
            stat_p.push_back(vector<double>());
            stat_u.push_back(vector<double>());
            stat_B.push_back(vector<double>());
            stat_C.push_back(vector<arma::vec>());
            stat_W.push_back(vector<arma::vec>());
        }
        a.fill(1.0);
    }

    void simdt(const double &t, const double &dt, const List &c, NetSim &n)  {
        arma::vec coins(N, arma::fill::randu);
        for(size_t ni=0; ni<N; ni++) {
            arma::uvec fired(id_conns[ni].n_elem, arma::fill::zeros);
            for(size_t  syn_i=0; syn_i < id_conns[ni].n_elem; syn_i++) {
                int num_spikes = n.getNumSpikes( id_conns[ni](syn_i), t, dt);
                if(num_spikes > 0) {
                    syn[ni](syn_i) += num_spikes*syn_spec[ni](syn_i);
                    fired(syn_i) = 1;
                }
                syn[ni](syn_i) *= a(ni);
            }
            double u = asD("u_rest", c) + sum(syn[ni] % W[ni]);
            double p = probf(u, c)*dt;
            bool Yspike = false;
            if(p > coins(ni)) {
                n.push_back(ids(ni), t);
                a(ni) = 0;
                Yspike = true;
            }
            pacc(ni) += p;
            C[ni] += -C[ni]/as<double>(c["tc"]) + C_calc(Yspike, p, u, syn[ni], c);
            syn[ni] -= syn[ni]/asD("tm", c);
            a += (1-a)/asD("ta", c);
            
            double B = B_calc(Yspike, p, pacc(ni)/(incr+1), c);
            W[ni] += asD("added_lrate",c)*ratecalc(W[ni],c) % (C[ni]*B - asD("weight_decay_factor",c)*(fired % W[ni]) );
           
            if(saveStat) {
                stat_p[ni].push_back(p);
                stat_u[ni].push_back(u);
                stat_B[ni].push_back(B);
                stat_C[ni].push_back(C[ni]);
                stat_W[ni].push_back(W[ni]);
            }
        }
        incr++;
    }
    
    // consts 
    int N;
    arma::uvec ids;
    TVecNums W;
    TVecIDs id_conns;
    TVecNums syn_spec;

    // vars
    TVecNums syn;
    arma::vec a;
    TVecNums C;
    arma::vec pacc;
    int incr;

    // stat
    bool saveStat;
    TStatAcc stat_p;
    TStatAcc stat_u;
    TStatAcc stat_B;
    TVecStatAcc stat_C;
    TVecStatAcc stat_W;
};

class SIM {
public:
    SIM() {}
    void addLayer(SRMLayer &l) {
        layers.push_back(&l);
    }
    void sim(const List sim_options, const List constants, List net) {
        const double T0 = as<double>(sim_options["T0"]);
        const double Tmax = as<double>(sim_options["Tmax"]);
        const double dt = as<double>(sim_options["dt"]);    
        const bool saveStat = as<bool>(sim_options["saveStat"]);    
        const bool learn = as<bool>(sim_options["learn"]);    
        arma::vec T = arma::linspace(T0, Tmax, (Tmax-T0)/dt + 1);

        int num_neurons = 0;
        for(size_t li=0; li<layers.size(); li++) {
            layers[li]->prepare(constants);
            layers[li]->saveStat = saveStat;
            num_neurons += layers[li]->N;
        }
        if(num_neurons > net.size()) {
           ::Rf_error( "net list is less than size of layers\n");
        }
        NetSim ns(net);
        for(size_t ti=0; ti<T.n_elem; ti++) {
            for(size_t li=0; li<layers.size(); li++) {
                layers[li]->simdt(T(ti), dt, constants, ns);
            }
        }
    }
    vector<SRMLayer*> layers;
};

namespace Rcpp {
    template <> SEXP wrap<SRMLayer>( const SRMLayer& el ) {
            Rcpp::Language call( "new", Symbol( "SRMLayer" ), el) ;
            return call.eval();
    };

    template <> SRMLayer as<SRMLayer>( SEXP s )  { // throw(not_compatible) {
            try {
                if ( TYPEOF(s) != S4SXP ) {
                    ::Rf_error( "supplied object is not of type SRMLayer." );
                }

                Rcpp::S4 s4obj( s );
                if ( !s4obj.is("Rcpp_SRMLayer" ) ) {
                    ::Rf_error( "supplied object is not of type SRMLayer." );
                }

                Rcpp::Environment env( s4obj );
                Rcpp::XPtr<SRMLayer> xptr( env.get(".pointer") );

                    return SRMLayer(xptr->N, xptr->ids, xptr->W, xptr->id_conns, xptr->syn, xptr->syn_spec, xptr->a, xptr->C, xptr->pacc, xptr->incr );
            }
            catch(...) {
                ::Rf_error( "supplied object could not be converted to SRMLayer." );
            }
    };
};

RCPP_MODULE(snnMod){
    class_<SRMLayer>( "SRMLayer" )
    .constructor<size_t>()
    .field("N", &SRMLayer::N, "Number of neurons")
    .field("ids", &SRMLayer::ids, "Unique ids of neurons")
    .field("a", &SRMLayer::a, "Neurons refr")
    .field("W", &SRMLayer::W, "Weights")
    .field("C", &SRMLayer::C, "C")
    .field("id_conns", &SRMLayer::id_conns, "IDs of connections")
    .field("syn", &SRMLayer::syn, "synapses")
    .field("syn_spec", &SRMLayer::syn_spec, "synapse specializations")
    .field("stat_p", &SRMLayer::stat_p, "Statistics of probs")
    .field("stat_u", &SRMLayer::stat_u, "Statistics of pots")
    .field("stat_B", &SRMLayer::stat_B, "Statistics of B")
    .field("stat_C", &SRMLayer::stat_C, "Statistics of C")
    .field("stat_W", &SRMLayer::stat_W, "Statistics of W")
    .field("pacc", &SRMLayer::pacc, "Accum for p")
    .field("incr", &SRMLayer::incr, "Increment of simulations")
    .method("prepare", &SRMLayer::prepare)
    ;
    class_<SIM>( "SIM" )
    .constructor()
    .method("addLayer", &SIM::addLayer)
    .method("sim", &SIM::sim)
    ;
}

