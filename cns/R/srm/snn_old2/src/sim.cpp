

#include "neuron_funcs.h"
#include "sim.h"
#include "inp_neurons.cpp"
#include "layer.h"
#include "netsim.h"


double asD(const char *name, const List &c) {
    return as<double>(c[name]);
}


//#define FINITE_CHECK

#define SYN_ACT_TOL 0.001

class SRMLayer : public Layer {
public:
    SRMLayer(int &N_, arma::uvec &ids_, TVecNums &W_, arma::vec &Ws4_, TVecIDs &id_conns_, TVecNums &syn_, TVecNums &syn_spec_, arma::vec &a_, TVecNums &C_, arma::vec &B_, arma::vec &pacc_, int incr_, vector< set<size_t> > active_syns_) : 
                        Layer(N_),
                        ids(ids_),
                        W(W_),
                        Ws4(Ws4_),
                        id_conns(id_conns_),
                        syn(syn_),
                        syn_spec(syn_spec_),
                        a(a_),
                        C(C_),
                        B(B_),
                        pacc(pacc_),
                        incr(incr_),
                        active_syns(active_syns_)
    {}
    SRMLayer(const SRMLayer &l) : Layer(l.num()), ids(l.ids), W(l.W), Ws4(l.Ws4), id_conns(l.id_conns), syn(l.syn), syn_spec(l.syn_spec), a(l.a), C(l.C), B(l.B), pacc(l.pacc), incr(l.incr), active_syns(l.active_syns) {}
    SRMLayer(int N_) : Layer(N_), ids(N), a(N, arma::fill::zeros), C(N), B(N), W(N), Ws4(N, arma::fill::zeros), id_conns(N), syn(N), syn_spec(N), pacc(N, arma::fill::zeros), incr(0), active_syns(N_) { }
    const int num() const {
        return N;
    }
    void setNum(int N_) {
        N = N_;
    }
    const arma::uvec& getConns(size_t ni) const {
        return id_conns[ni];
    }
    const arma::uvec& getIds() const {
        return ids;
    }

    void prepare(const List &c) {
        stat_p.clear(); stat_u.clear(); stat_B.clear(); stat_C.clear(); stat_W.clear(); 
        for(size_t ni=0; ni < N; ni++) {
            if(syn[ni].n_elem == 0) {
                syn[ni] = arma::vec(id_conns[ni].n_elem, arma::fill::zeros);
            } else {
                syn[ni].fill(0.0);
            }
            Ws4(ni) = asD("weight_per_neuron", c)/id_conns[ni].n_elem;
            C[ni].fill(0.0);
            stat_p.push_back(vector<double>());
            stat_u.push_back(vector<double>());
            stat_B.push_back(vector<double>());
            stat_C.push_back(vector<arma::vec>());
            stat_W.push_back(vector<arma::vec>());
        }
        Ws4 = arma::pow(Ws4, 4);
        a.fill(1.0);
        B.fill(0.0);
    }

    void simdt(const double &t, const double &dt, const List &c, NetSim &n)  {
        arma::vec coins(N, arma::fill::randu);
        for(size_t ni=0; ni<N; ni++) {
            arma::uvec fired(id_conns[ni].n_elem, arma::fill::zeros);
            double u = asD("u_rest", c);
            
            set<size_t>::iterator it;
            for (it = active_syns[ni].begin(); it != active_syns[ni].end(); ) {
                syn[ni]( *it ) *= a(ni);
                u += W[ni]( *it )*syn[ni]( *it );
                if(abs(syn[ni]( *it )) < SYN_ACT_TOL) {
                    active_syns[ni].erase(it++);
                } else {
                    ++it;
                }
            }

            TSynSpikes ssp = n.getSpikes(ids(ni), t, dt );
            for(size_t syn_sp_i=0; syn_sp_i<ssp.size(); syn_sp_i++) {
//                cout << "t:" << t << " We have spike at " << ids(ni)-1 << " at synapse " << ssp[syn_sp_i].syn_id << " at t = " << ssp[syn_sp_i].t << "\n"; 
                syn[ni]( ssp[syn_sp_i].syn_id ) += syn_spec[ni]( ssp[syn_sp_i].syn_id )*asD("e0",c);
                u += W[ni]( ssp[syn_sp_i].syn_id )*syn[ni]( ssp[syn_sp_i].syn_id );
                active_syns[ni].insert( ssp[syn_sp_i].syn_id );
                fired(ssp[syn_sp_i].syn_id) = 1;
            }
            
            bool Yspike = false;
            double p = 0;
            if(!determ) {
                p = probf(u, c)*dt;
//                if(p > 1000) {
//                    cout << "Something wrong: big p\n";
//                    cout << "neuron id " << ids(ni) << "\n";
//                    cout << "p: " << p << "\n";
//                    cout << "u: " << u << "\n";
//                    cout << "fired: \n";
//                    fired.t().print();
//                    for(size_t syn_i=0; syn_i < id_conns[ni].n_elem; syn_i++) {
//                        int num_spikes = n.getNumSpikes( id_conns[ni](syn_i), t-syn_del[ni](syn_i));
//                        cout << "num_spikes: " << num_spikes << "\n";
//                        if(num_spikes > 0) {
//                            cout << "syn_spec[" << ni << "](" << syn_i << ") = " << syn_spec[ni](syn_i) << ", e0 = " << asD("e0",c) << "\n";
//                        }
//                        cout << "a(" << ni << ") = " << a(ni) << "\n";
//                        cout << "u+ =" << W[ni](syn_i) << " * " << syn[ni](syn_i) << "\n";
//                    }
//                    ::Rf_error("error");
//
//                }
                if(p > coins(ni)) {
                    Yspike = true;
                }
            } else {
                if(u >= asD("tr",c)) {
                    Yspike = true;
                }            
            }
            
            if(Yspike) {
                n.prop_spike(ids(ni), t+dt+axon_del(ni));
                a(ni) = 0;
                if(!determ) pacc(ni) += 1;
            }
            
            // common dynamics 
            if(!determ) pacc(ni) += -pacc(ni)/(asD("mean_p_dur",c));
            syn[ni] -= syn[ni]/asD("tm", c);
            a += (1-a)/asD("ta", c);
            
            if (incr>=asD("mean_p_dur",c)&&(!determ)) {
                B(ni) = B_calc(Yspike, p, pacc(ni)/asD("mean_p_dur",c), c);
                C[ni] += -C[ni]/as<double>(c["tc"]) + C_calc(Yspike, p, u, syn[ni], c);
                arma::vec dw = asD("added_lrate",c)*ratecalc(W[ni],Ws4(ni)) % (C[ni]*B(ni) - asD("weight_decay_factor",c)*(fired % W[ni]) );
                if(learn) W[ni] += dw;

#ifdef FINITE_CHECK            
                if(!arma::is_finite(dw)) {
                    cout << "Found infinity in dw, for neuron " << ni << "\n";
                    cout << "added_lrate = " << asD("added_lrate",c) << " ratecalc(W[ni]) = \n"; 
                    ratecalc(W[ni],Ws4(ni)).t().print();
                    cout << "C[ni] = " << "\n";
                    C[ni].t().print();
                    cout << "B = " << B(ni) << " Yspike = " << Yspike  << " u = " << u << " p = " <<  p << " pm = " << pacc(ni)/asD("mean_p_dur",c) <<  "\n";
                    cout<< " weight decay: " << asD("weight_decay_factor",c) << "\n";
                    cout << "fired: \n";
                    fired.t().print();
                    cout << "W[ni] = \n";
                    W[ni].t().print();
                    ::Rf_error("error");
                }
#endif                
            }
            if(saveStat) {
                stat_p[ni].push_back(p);
                stat_u[ni].push_back(u);
                stat_B[ni].push_back(B(ni));
                stat_C[ni].push_back(C[ni]);
                if(learn)
                    stat_W[ni].push_back(W[ni]);
            }            
        }
        incr+=dt;
    }
    // consts 
    arma::uvec ids;
    TVecNums W;
    TVecIDs id_conns;
    TVecNums syn_spec;
    TVecNums syn_del;
    arma::vec axon_del;
    arma::vec Ws4;

    // vars
    TVecNums syn;
    arma::vec a;
    TVecNums C;
    arma::vec B;
    arma::vec pacc;
    double incr;

    vector< set<size_t> > active_syns;
    
    TVecAcc stat_p;
    TVecAcc stat_u;
    TVecAcc stat_B;
    TVecArr stat_C;
    TVecArr stat_W;
};

class SIM {
public:
    SIM() {}
    void addSRMLayer(SRMLayer &l) {
        layers.push_back(&l);
    }
    void addFBLayer(FBLayer &l) {
        layers.push_back(&l);
    }

    void sim(const List sim_options, const List constants, Reference net_ref) {
        const double dt = as<double>(sim_options["dt"]);    
        const bool saveStat = as<bool>(sim_options["saveStat"]);    
        const bool learn = as<bool>(sim_options["learn"]);    
        const bool determ = as<bool>(sim_options["determ"]);    
        NumericVector pattTimeline = net_ref.field("timeline");
        List net = net_ref.field("net");
        double T0 = 0;
        double Tmax = as<NumericVector>(net_ref.field("Tmax"))[0];

        arma::vec T = arma::linspace(T0, Tmax, (Tmax-T0)/dt);
        if(determ && learn) {
            ::Rf_error( "Net can't learn being in detemenitic mode" );
        }
        int num_neurons = 0;
        for(size_t li=0; li<layers.size(); li++) {
            layers[li]->prepare(constants);
            layers[li]->saveStat = saveStat;
            layers[li]->learn = learn;
            layers[li]->determ = determ;
            num_neurons += layers[li]->N;
        }
        if(num_neurons > net.size()) {
           ::Rf_error( "net list is less than size of layers\n");
        }
        NetSim ns(net, layers);
//        for(size_t nc=0; nc < ns.cons.size(); nc++) {
//            for(size_t ci=0; ci < ns.cons[nc].size(); ci++) {
//                cout << "Neuron with id " << nc + 1 << " will cause spikes in " << ns.cons[nc][ci].first << " on synapse id " << ns.cons[nc][ci].second << "\n";  
//            }
//        }
        size_t patt_id = 0;
        bool refreshNet = false;
        for(size_t ti=0; ti<T.n_elem; ti++) {
            if((pattTimeline[patt_id] >= T(ti))&&(pattTimeline.size() < patt_id) ) {
                refreshNet = true;
            }
            for(size_t li=0; li<layers.size(); li++) {
                layers[li]->simdt(T(ti), dt, constants, ns);
                if(refreshNet) { 
                    layers[li]->prepare(constants); 
                }
            }
            if(refreshNet) {
                patt_id++;
                refreshNet = false;
            }
            
        }
    }

    vector<Layer*> layers;
};

#include "rcpp_wrap.h"

RCPP_MODULE(snnMod){
    class_<SRMLayer>( "SRMLayer" )
    .constructor<size_t>()
    .method("num", &SRMLayer::num)
    .method("setNum", &SRMLayer::setNum)
    .field("ids", &SRMLayer::ids, "Unique ids of neurons")
    .field("a", &SRMLayer::a, "Neurons refr")
    .field("W", &SRMLayer::W, "Weights")
    .field("C", &SRMLayer::C, "C")
    .field("id_conns", &SRMLayer::id_conns, "IDs of connections")
    .field("syn", &SRMLayer::syn, "synapses")
    .field("syn_spec", &SRMLayer::syn_spec, "synapse specializations")
    .field("syn_del", &SRMLayer::syn_del, "synapse delays")
    .field("axon_del", &SRMLayer::axon_del, "axon delays")
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
    .method("addSRMLayer", &SIM::addSRMLayer)
    .method("addFBLayer", &SIM::addFBLayer)
    .method("sim", &SIM::sim)
    ;
    class_<FBLayer>("FBLayer")
    .constructor<size_t>()
    .method("num", &FBLayer::num)
    .field("a", &FBLayer::a)
    .field("w", &FBLayer::w)
    .field("v", &FBLayer::v)
    .field("I", &FBLayer::I)
    .field("stat_v", &FBLayer::stat_v)
    .field("stat_w", &FBLayer::stat_w)
    ;
}

