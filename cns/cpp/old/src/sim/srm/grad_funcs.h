#ifndef GRAD_FUNCS_H
#define GRAD_FUNCS_H

#include <sim/core.h>

namespace srm {
    class SrmNeuron;
    class TTime;
    
    double grab_epsp_syn(const double &t, const int &j, SrmNeuron *n, TTime &y);

    double p_stroke(double t, SrmNeuron *n, TTime &y);
   
    double integrand_epsp_gl(double t, void* data);

    struct TNeuronSynapseGivenY { 
        TNeuronSynapseGivenY(size_t &ind_syn_v, SrmNeuron* n_v, TTime &y_v);
        size_t ind_syn;
        SrmNeuron *n;
        TTime &y;
    };            

};

#endif
