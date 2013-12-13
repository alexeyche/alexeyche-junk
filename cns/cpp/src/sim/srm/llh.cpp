

#include "grad_funcs.h"
#include "llh.h"
#include "neurons.h"
#include "cfg.h"

#include <sim/int/gauss_legendre.h>
#include <sim/socket/sim_socket_core.h>

namespace srm {

    vec TLogLikelyhood::grad() {
        vec grad_val(n->w.size());
    #if VERBOSE >= 2
        printf("llh grad neuron %d\n", n->id());
    #endif
        for(size_t wi=0; wi<grad_val.n_elem; wi++) {
            TTime null;
            TNeuronSynapseGivenY n_syn_y(wi, n, null); // n->y);       
            double int_part = - gauss_legendre(GAUSS_QUAD, integrand_epsp_gl, (void*)&n_syn_y, T0, Tmax);           
                            
        #if VERBOSE >= 3
            printf(" | syn # %zu    ----          spikes: [", wi);
            for(size_t starti=n->in[wi]->y.n_elem(T0); starti<n->in[wi]->y.n_elem(Tmax); starti++) {
                printf("%f, ", n->in[wi]->y[starti]);
            }
            printf("]\n");
            printf(" | int part :%f\n", int_part);
        #endif
//            if(n->in[wi]->y.n_elem(Tmax) > 0) 
//            if(n->y[0]< n->in[wi]->y[0]) { 
//                mat ps_ep(1000,3);
//                vec t = linspace<vec>(T0,Tmax, 1000);
//                TTime null;
//                for(size_t ti=0; ti<t.n_elem; ti++) {
//                    ps_ep(ti,0) = t(ti);
//                    ps_ep(ti,1) = p_stroke(t(ti), n, null);
//                    ps_ep(ti,2) = grab_epsp_syn(t(ti), wi, n, null);
//                }
//                Log::Info << "sending data for [" << T0 << "," << Tmax << "]\n";
//                send_arma_mat(ps_ep,"ps_ep", -1, true);
//                throw SrmException("QuieeeeeT\n");
//            }           
            double spike_part = 0;
            for(size_t yi=n->y.n_elem(T0); yi<n->y.n_elem(Tmax); yi++) {
                double &fi = n->y[yi];
                spike_part += (p_stroke(fi, n, n->y)/n->p(fi))*grab_epsp_syn(fi, wi, n, n->y);
            #if VERBOSE >= 3
                printf(" | yi: %d\n", yi);
                printf("   | p' = %f\n", p_stroke(fi, n, n->y));
                printf("   | p = %f\n", n->p(fi));
                printf("   | u(t) = %f\n", n->u(fi));
                printf("   | epsp = %f\n", grab_epsp_syn(fi, wi, n, n->y));
                printf("   | y[yi] = %f\n", fi);
                printf("   | spike_part %f\n", spike_part);
            #endif
//                break;
            }            
            grad_val(wi) = int_part + spike_part;
        }
    #if VERBOSE >= 2
        printf("mean grad(%f, %f) = %f\n", T0, Tmax, mean(grad_val(find(grad_val != 0))));
    #endif
        return grad_val;
    }

};
