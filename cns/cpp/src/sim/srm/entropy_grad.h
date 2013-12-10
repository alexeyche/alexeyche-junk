#ifndef ENTROPY_GRAD_H
#define ENTROPY_GRAD_H

#include "research.h"
#include "entropy.h"
#include "grad_funcs.h"

namespace srm {
    class TEntropyGrad : public TEntropyCalc {
    public:
        TEntropyGrad(SrmNeuron *n_v, double T0_v, double Tmax_v) : TEntropyCalc(n_v, T0_v, Tmax_v) {} 

        // one numerical evaluation of EntropyGrad
        static vec EntropyGradGivenY(TEntropyGrad *eg, TTime &y) {
            double p = survFunction(eg->neuron, y, eg->T0, eg->Tmax);
            vec grad(eg->neuron->w.size());
            for(size_t wi=0; wi< grad.n_elem; wi++) {
                TNeuronSynapseGivenY n_syn_y(wi, eg->neuron, y);           
                double int_part = gauss_legendre(eg->cs.GaussQuad, integrand_epsp_gl, (void*)&n_syn_y, eg->T0, eg->Tmax);
                if(eg->cs.VerboseInt) printf("int_part wi(%zu) = %f", wi, int_part); 
                double spike_part = 0;
                for(size_t yi=0; yi<y.size(); yi++) {
                    double &fi = y[yi];
                    spike_part += (p_stroke(fi, eg->neuron, y)/eg->neuron->p(fi, y))*grab_epsp_syn(fi, wi, eg->neuron, y);
                    if(eg->cs.VerboseInt) {
                        printf(" | yi: %zu ", yi);
                    //    printf("   | p' = %f\n", p_stroke(fi, eg->neuron, y));
                    //    printf("   | p = %f\n", eg->neuron->p(fi, y));
                    //    printf("   | u(t) = %f\n", eg->neuron->u(fi, y));
                    //    printf("   | y[yi] = %f\n", y[yi]);
                        printf("   | spike_part %f\n", spike_part);
                    }                                           
                }
                grad(wi) = -p*(log(p)+1)*(int_part + spike_part);
            }
            return grad;
        }

        // for Monte Carlo integration 
        static int EntropyGradIntegrand(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata) {
            TEntropyGrad *eg = (TEntropyGrad*)userdata;
            TTime y(*ndim);
            double t_cur= -datum::inf;
            for(size_t nd = 0; nd< *ndim; nd++) {
                y[nd] = eg->T0 + (eg->Tmax - eg->T0)*xx[nd];
                if(t_cur > y[nd]) {
                    for(size_t ni=0; ni < *ncomp; ni++) { ff[ni] = 0; }
                    return 0;
                } 
                t_cur = y[nd];
            }
            double p = survFunction(eg->neuron, y, eg->T0, eg->Tmax);
            if(eg->cs.VerboseInt) {
                printf("survFunction for y = [ ");
                for(size_t nd=0; nd< y.size(); nd++) {
                    printf("%f, ", y[nd]);
                }
                printf("] = %e  \n", p);
            }
            for(size_t wi=0; wi< *ncomp; wi++) {
                TNeuronSynapseGivenY n_syn_y(wi, eg->neuron, y);           
                double int_part = gauss_legendre(eg->cs.GaussQuad, integrand_epsp_gl, (void*)&n_syn_y, eg->T0, eg->Tmax);
                if(eg->cs.VerboseInt) printf("int_part wi(%zu) = %f", wi, int_part); 
                double spike_part = 0;
                for(size_t yi=0; yi<y.size(); yi++) {
                    double &fi = y[yi];
                    spike_part += (p_stroke(fi, eg->neuron, y)/eg->neuron->p(fi, y))*grab_epsp(fi, wi, eg->neuron, y);
                    if(eg->cs.VerboseInt) {
                        printf(" | yi: %zu ", yi);
                    //    printf("   | p' = %f\n", p_stroke(fi, eg->neuron, y));
                    //    printf("   | p = %f\n", eg->neuron->p(fi, y));
                    //    printf("   | u(t) = %f\n", eg->neuron->u(fi, y));
                    //    printf("   | y[yi] = %f\n", y[yi]);
                        printf("   | spike_part %f\n", spike_part);
                    }                                           
                }
                ff[wi] = -p*(log(p)+1)*(int_part + spike_part);
                ff[wi] = ff[wi]*(eg->Tmax - eg->T0);
            }
             
            return 0;
        }
        vec grad_1eval() {
            vec w_grad(neuron->w.size(), fill::zeros);
            TTime y_no_spikes = TTime();
            double p0 = survFunction(neuron, y_no_spikes, T0, Tmax);
            for(size_t wi=0; wi < neuron->w.size(); wi++) {
                TNeuronSynapseGivenY p(wi, neuron, y_no_spikes);
                double sec_part = gauss_legendre(cs.GaussQuad, integrand_epsp_gl, (void*)&p, T0, Tmax);
                w_grad(wi) += -p0*(log(p0)+1)*sec_part;
//                Log::Info << "Hgrad0[" << wi << "] = " << w_grad(wi) << "\n";
            }           
            w_grad += EntropyGradGivenY(this, neuron->y);
            return w_grad;
        }
        vec grad() {
            vec w_grad(neuron->w.size(), fill::zeros);
            // for no spikes:
            TTime y_no_spikes = TTime();
            double p0 = survFunction(neuron, y_no_spikes, T0, Tmax);
            for(size_t wi=0; wi < neuron->w.size(); wi++) {
                TNeuronSynapseGivenY p(wi, neuron, y_no_spikes);
                double sec_part = gauss_legendre(cs.GaussQuad, integrand_epsp_gl, (void*)&p, T0, Tmax);
                w_grad(wi) += -p0*(log(p0)+1)*sec_part;
                //Log::Info << "Hgrad0[" << wi << "] = " << w_grad(wi) << "\n";
            }
            for(int n_calc = 1; n_calc <= 2; n_calc++) {
                               
                vec integral = integrate(n_calc, neuron->w.size(), EntropyGradIntegrand);
                w_grad += integral;
                //for(size_t wi=0; wi<neuron->w.size(); wi++) {
                //    Log::Info << "Hgrad" << n_calc << "[" << wi << "] = " << integral[wi] << "\n";
                //}
            }
            return w_grad;
        }
    };
};   

#endif
