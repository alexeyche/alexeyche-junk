
#include "grad_funcs.h"
#include "neurons.h"

namespace srm {
    double grab_epsp_syn(const double &t, const int &j, SrmNeuron *n, TTime &y) {
        double pot_j = 0;
        Neuron *n_j = n->in[j];
        double &y_last = y.last(t-0.01);
        for(int yi = n_j->y.n_elem(t)-1; yi>=0; yi--) {
           if( (t - n_j->y(yi)) > EPSP_WORK_WINDOW) { break; }
           pot_j += SrmNeuron::epsp(t, n_j->y(yi), y_last);
        }
        return pot_j;
    }

    double p_stroke(double t, SrmNeuron *n, TTime &y) {
        return SrmNeuron::beta/( 1 + exp(SrmNeuron::alpha*(SrmNeuron::tresh - n->u(t, y))) );
    }
    

    TNeuronSynapseGivenY::TNeuronSynapseGivenY(size_t &ind_syn_v, SrmNeuron* n_v, TTime &y_v) : ind_syn(ind_syn_v), n(n_v), y(y_v) {}

    double integrand_epsp_gl(double t, void* data) {
        TNeuronSynapseGivenY *p = (TNeuronSynapseGivenY*)data;
//        printf("int debug(%f): p'= %f epsp= %f\n", t, p_stroke(t, p->n, p->y), grab_epsp_syn(t, p->ind_syn, p->n, p->y));
        return p_stroke(t, p->n, p->y)*grab_epsp_syn(t, p->ind_syn, p->n, p->y);
    }   
};
