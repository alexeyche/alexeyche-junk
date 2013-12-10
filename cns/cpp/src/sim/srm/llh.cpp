

#include "grad_funcs.h"
#include "llh.h"
#include "neurons.h"
#include "cfg.h"

#include <sim/int/gauss_legendre.h>

namespace srm {

    vec TLogLikelyhood::grad() {
        vec grad_val(n->w.size());
    #if VERBOSE >= 2
        printf("llh grad neuron %d\n", n->id());
    #endif
        for(size_t wi=0; wi<grad_val.n_elem; wi++) {
            TNeuronSynapseGivenY n_syn_y(wi, n, n->y);       
            double int_part = gauss_legendre(GAUSS_QUAD, integrand_epsp_gl, (void*)&n_syn_y, T0, Tmax);           
        
        #if VERBOSE >= 3
            printf(" | syn # %zu    ----          spikes: [", wi);
//            printf(" DEBUG %d %d %d %d ", n->in[wi]->y.binary_search(T0), n->in[wi]->y.binary_search(Tmax), n->in[wi]->y.n_elem(T0), n->in[wi]->y.n_elem(Tmax)); 
            
            for(size_t starti=0; starti<n->in[wi]->y.size(); starti++) {
                printf("%f, ", n->in[wi]->y[starti]);
            }
            printf("]\n");
            printf(" | int part :%f\n", int_part);
        #endif
            double spike_part = 0;
            int start_i = n->y.n_elem(T0);
            for(int yi=start_i; yi<n->y.n_elem(Tmax); yi++) {
                double &fi = n->y[yi];
                spike_part += (p_stroke(fi, n, n->y)/n->p(fi))*grab_epsp_syn(fi, wi, n, n->y);
            #if VERBOSE >= 3
                printf(" | yi: %zu\n", yi);
                printf("   | p' = %f\n", p_stroke(fi, n, n->y));
                printf("   | p = %f\n", n->p(fi));
                printf("   | u(t) = %f\n", n->u(fi));
                printf("   | epsp = %f\n", grab_epsp_syn(fi, wi, n, n->y));
                printf("   | y[yi] = %f\n", fi);
                printf("   | spike_part %f\n", spike_part);
            #endif
            }            
            grad_val(wi) = int_part + spike_part;
        }
    #if VERBOSE >= 2
        printf("mean grad(%f, %f) = %f\n", T0, Tmax, mean(grad_val));
    #endif
        return grad_val;
    }

};
