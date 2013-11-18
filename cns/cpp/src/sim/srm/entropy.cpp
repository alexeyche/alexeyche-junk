#include <sim/int/simple_int.h>


#include "entropy.h"
#include "research.h"

#include <cuba.h>

namespace srm {

    int EntropyCalc::Integrand(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata) {
        EntropyCalc *ec = (EntropyCalc*)userdata;
        double t_cur= -datum::inf;
        for(size_t ni=0; ni<ec->n; ni++) {
            ec->neuron->y[ni] = ec->T0 + (ec->Tmax - ec->T0)*xx[ni];
            if(t_cur > ec->neuron->y[ni]) {
                ff[0] = 0;
                return 0;
            } 
            t_cur = ec->neuron->y[ni];
        }
        double p = survFunction(ec->neuron, ec->T0, ec->Tmax);

        ff[0] = p*log(p);

        if(ec->cuba_verbose>0) {
            printf("survFunction for y = [ ");
            for(size_t ni=0; ni<ec->n; ni++) {
                printf("%f, ", ec->neuron->y[ni]);
            }
            printf("] = %f \n",ff[0]);
        }

        ff[0] = ff[0]*(ec->Tmax - ec->T0);
        return 0;
    }
//    double EntropyCalc::entropy_fn_int(const double &fn, EntropyCalc *ec) {
//        if(ec->n_cur < ec->n-1) {
//            ec->neuron->y[ec->n_cur] = fn;
//            ec->n_cur += 1;
//            Log::Info << "We going to integrate entr [" << fn << "," << ec->Tmax << "]\n";
//            double H = int_trapezium<EntropyCalc>(fn, ec->Tmax, 100, ec, &entropy_fn_int);
////            double H = DEIntegrator<double, EntropyCalc>::Integrate(ec, &entropy_fn_int, fn, ec->Tmax, 1e-06);
//            ec->n_cur = 0;
//            Log::Info << "We integrated H = " << H << "\n"; 
//            return H;
//        } else {
//            ec->neuron->y[ec->n_cur] = fn;
//            double p = survFunction(ec->neuron, ec->T0, ec->Tmax);
//            Log::Info << "survFunction for y = [ ";
//            for(size_t ni=0; ni<ec->n; ni++) {
//                Log::Info << ec->neuron->y[ni] << ", ";
//            }
//            Log::Info << " ] = " << p << "\n";
//            if(p == 0) {
//                return 0;
//            }
//            return p*log(p);
//        }            
//    }

    double EntropyCalc::run(size_t n_calc) {
        neuron->y.clean();
        for(size_t ni=0; ni<n_calc; ni++) {
            neuron->y.push_back(T0);
        }
        int verbose, comp, nregions, neval, fail;
        double integral, error, prob;
        n = n_calc;
        if(n_calc == 0) {
            n_calc = 1;
        }
        Vegas(n_calc, 1, Integrand, this, 1e-3, 1e-12, cuba_verbose, 0, 0, 30000, 1000, 500, 100, 0, NULL, &neval, &fail, &integral, &error, &prob);
        if(cuba_verbose>0) {
            printf("VEGAS RESULT:\tneval %d\tfail %d\n", neval, fail);
            printf("VEGAS RESULT:\t%.8f +- %.8f\tp = %.3f\n", integral, error, prob);
        }            
        return integral;
    }
};    

