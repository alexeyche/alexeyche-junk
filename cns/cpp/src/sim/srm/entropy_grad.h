#ifndef ENTROPY_GRAD_H
#define ENTROPY_GRAD_H

#include "research.h"
#include "entropy.h"

namespace srm {
    class TEntropyGrad : public EntropyCalc {
    public:
        TEntropyGrad(SrmNeuron *n_v, double T0_v, double Tmax_v) : EntropyCalc(n_v, T0_v, Tmax_v) {} 
        static double p_stroke(double &t, SrmNeuron *n) {
            return SrmNeuron::beta/( 1 + exp(SrmNeuron::alpha*(SrmNeuron::tresh - n->u(t))) );
        }
        static double grab_epsp(const double &t, const int &j, SrmNeuron *n) {
            double pot_j = 0;
            Neuron *n_j = n->in[j];
            double &y_last = n->y.last(t);
            for(int yi = n_j->y.n_elem(t)-1; yi>=0; yi--) {
                pot_j += SrmNeuron::epsp(t, n_j->y[yi], y_last);
            }
            return pot_j;
        }
        
        // for [0, T] integration with Gauss Quadrature
        static double integrand_epsp_gl(double t, void* data) {
            std::pair<int, SrmNeuron*> *p = (std::pair<int, SrmNeuron*>*)data;
            return -p_stroke(t, p->second) * grab_epsp(t, p->first, p->second);
        }
        
        // for Monte Carlo integration 
        static int integrand_grad_no_spike(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata) {
            TEntropyGrad *eg = (TEntropyGrad*)userdata;
            double t_cur= -datum::inf;
            for(size_t nd = 0; nd<eg->n; nd++) {
                eg->neuron->y[nd] = eg->T0 + (eg->Tmax - eg->T0)*xx[nd];
                if(t_cur > eg->neuron->y[nd]) {
                    ff[0] = 0;
                    return 0;
                } 
                t_cur = eg->neuron->y[nd];
            }
            double p0 = survFunction(eg->neuron, eg->T0, eg->Tmax);
            std::pair<int, SrmNeuron*> p(0, eg->neuron);           
            double sec_part = gauss_legendre(128, integrand_epsp_gl, (void*)&p, eg->T0, eg->Tmax);

            ff[0] = p0*(log(p0)+1)*sec_part;
            
            if(eg->cs.VerboseInt) {
                printf("survFunction for y = [ ");
                for(size_t nd=0; nd< eg->neuron->y.size(); nd++) {
                    printf("%f, ", eg->neuron->y[nd]);
                }
                printf("] = %e  H grad = %e\n", p0, ff[0]); 
            }           
            
            ff[0] = ff[0]*(eg->Tmax - eg->T0);
            return 0;
        }

        double grad() {
            neuron->y.clean();
            double p0 = survFunction(eg->neuron, eg->T0, eg->Tmax);
            std::pair<int, SrmNeuron*> p(0, eg->neuron);           
            double sec_part = gauss_legendre(128, integrand_epsp_gl, (void*)&p, eg->T0, eg->Tmax);
            double Hgrad_0 = p0*(log(p0)+1)*sec_part;
            Log::Info << "Hgrad_0: " << Hgrad_0 << "\n";
            int verbose, comp, nregions, neval, fail;
            double integral, error, prob;           

            n=1;
            int n_calc = 1; 
            cs.MaxEval = 3000; 
            cs.EpsAbs = 1e-06;
            Vegas(n_calc, 1, integrand_grad_no_spike, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.NStart, cs.NIncrease, cs.NBatch, cs.GridNo, NULL, &neval, &fail, &integral, &error, &prob);
            return integral; 
        }
    };
};   

#endif
