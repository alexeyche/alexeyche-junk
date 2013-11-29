#ifndef ENTROPY_GRAD_H
#define ENTROPY_GRAD_H

#include "research.h"
#include "entropy.h"

namespace srm {
    class TEntropyGrad : public EntropyCalc {
    public:
        TEntropyGrad(SrmNeuron *n_v, double T0_v, double Tmax_v) : EntropyCalc(n_v, T0_v, Tmax_v) {} 
        static double p_stroke(double t, SrmNeuron *n, TTime &y) {
            return SrmNeuron::beta/( 1 + exp(SrmNeuron::alpha*(SrmNeuron::tresh - n->u(t, y))) );
        }
        static double grab_epsp(const double &t, const int &j, SrmNeuron *n, TTime &y) {
            double pot_j = 0;
            Neuron *n_j = n->in[j];
            double &y_last = y.last(t-0.01);
            for(int yi = n_j->y.n_elem(t)-1; yi>=0; yi--) {
               if( (t - n_j->y(yi)) > EPSP_WORK_WINDOW) { break; }
               pot_j += SrmNeuron::epsp(t, n_j->y(yi), y_last);
            }
            return pot_j;
        }
        
        // for [0, T] integration with Gauss Quadrature
        struct TNeuronSynapseGivenY { 
            TNeuronSynapseGivenY(size_t &ind_syn_v, SrmNeuron* n_v, TTime &y_v) : ind_syn(ind_syn_v), n(n_v), y(y_v) {}
            size_t ind_syn;
            SrmNeuron *n;
            TTime &y;
        };            
        static double integrand_epsp_gl(double t, void* data) {
            TNeuronSynapseGivenY *p = (TNeuronSynapseGivenY*)data;
            return -p_stroke(t, p->n, p->y) * grab_epsp(t, p->ind_syn, p->n, p->y);
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
            for(size_t wi=0; wi< *ncomp; wi++) {
                TNeuronSynapseGivenY n_syn_y(wi, eg->neuron, y);           
                double int_part = gauss_legendre(128, integrand_epsp_gl, (void*)&n_syn_y, eg->T0, eg->Tmax);
                double spike_part = 0;
                for(size_t yi=0; yi<y.size(); yi++) {
                    double &fi = y[yi];
                    spike_part += (p_stroke(fi, eg->neuron, y)/eg->neuron->p(fi, y))*grab_epsp(fi, wi, eg->neuron, y);
                    if(eg->cs.VerboseInt) {
                        printf("yi: %zu\n", yi);
                        printf("   | p' = %f\n", p_stroke(fi, eg->neuron, y));
                        printf("   | p = %f\n", eg->neuron->p(fi, y));
                        printf("   | u(t) = %f\n", eg->neuron->u(fi, y));
                        printf("   | y[yi] = %f\n", y[yi]);
                        printf("   | spike_part %f\n", spike_part);
                    }                                           
                }
                ff[wi] = p*(log(p)+1)*(int_part + spike_part);
                ff[wi] = ff[wi]*(eg->Tmax - eg->T0);
            }
             
            return 0;
        }

        vec grad() {
            Log::Info << neuron->w.size() << "\n";
            vec w_grad(neuron->w.size(), fill::zeros);
            // for no spikes:
            TTime y_no_spikes = TTime();
            double p0 = survFunction(neuron, y_no_spikes, T0, Tmax);
            for(size_t wi=0; wi < neuron->w.size(); wi++) {
                TNeuronSynapseGivenY p(wi, neuron, y_no_spikes);
                double sec_part = gauss_legendre(128, integrand_epsp_gl, (void*)&p, T0, Tmax);
                w_grad(wi) += p0*(log(p0)+1)*sec_part;
                Log::Info << "Hgrad0[" << wi << "] = " << w_grad(wi) << "\n";
            }
            for(int n_calc = 1; n_calc <= 3; n_calc++) {
                               
                vec integral = integrate(n_calc, neuron->w.size(), EntropyGradIntegrand);
                w_grad += integral;
                for(size_t wi=0; wi<neuron->w.size(); wi++) {
                    Log::Info << "Hgrad" << n_calc << "[" << wi << "] = " << integral[wi] << "\n";
                }
            }
            return w_grad;
        }
    };
};   

#endif
