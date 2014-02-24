
#include "sim.h"
#include "inp_neurons.h"
#include "layer.h"


class FBLayer : public Layer {
public:
    FBLayer(int N_) : Layer(N_), a(N_, arma::fill::zeros), w(N_, arma::fill::zeros), v(N_, arma::fill::zeros) {
        Iiter=0;
    }
//    FBLayer(int N_, arma::vec a_, size_t Iiter_, arma::vec I_, arma::vec w_, arma::vec v_) : N(N_), a(a_), Iiter(Iiter_), I(I_), w(w_), v(v_) {}
    FBLayer(const FBLayer &l) : Layer(l.num()), a(l.a), w(l.w), v(l.v) {}
    const int num() const {
        return N;
    }
    void prepare(const List &c) {
        stat_v.clear(); stat_w.clear();
        Iiter = 0;
        for(size_t ni=0; ni<N; ni++) {
            stat_v.push_back(vector<double>());
            stat_w.push_back(vector<double>());
        }
    }
    void simdt(const double &t, const double &dt, const List &c, NetSim &n) {
        double Ival = 0;
        if(Iiter<I.n_elem) {
            Ival = I(Iiter);
        }
        v += dt*(-a % w -v + Ival)/(asD("tau",c));
        w += dt*(v-w)/(asD("tau",c));

        arma:: vec coins(N, arma::fill::randu);
        for(size_t ni=0; ni<N; ni++) {
            if( ((v(ni)*dt)) > coins(ni) ) {
                n.push_back(ni+1, t);
            }
            stat_v[ni].push_back(v(ni));
            stat_w[ni].push_back(w(ni));
        }
        
        Iiter++;
    }
    
    
    
    // consts:
    arma::vec a;
    size_t Iiter;
    arma::vec I;

    // vars:
    arma::vec w;
    arma::vec v;
    
    TStatAcc stat_v;
    TStatAcc stat_w;
};


