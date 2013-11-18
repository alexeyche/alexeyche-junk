#include <sim/int/simple_int.h>


#include "entropy.h"
#include "research.h"

#include <cuba.h>

namespace srm {
//    int EntropyCalc::Integrand(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata) {
//        EntropyCalc *ec = (EntropyCalc*)userdata;
//        double pp[NCOMP];
//        double t_cur= -datum::inf;
//        for(size_t nd = 0; nd<DIM_MAX; nd++) {
//            ec->neuron->y[nd] = ec->T0 + (ec->Tmax - ec->T0)*xx[nd];
//            if(t_cur > ec->neuron->y[nd]) {
//                for(size_t nni=0; nni<NCOMP; nni++) // wrong order
//                    ff[nni] = 0;
//                return 0;
//            } 
//            t_cur = ec->neuron->y[nd];
//        }                       
//        survFunctionSeq(ec->neuron, ec->T0, ec->Tmax, pp);
//        for(size_t ci=0; ci<NCOMP; ci++) {
//            ff[ci] = pp[ci]*log(pp[ci])*(ec->Tmax - ec->T0);           
//        }            
//        
//        //if(ec->cuba_verbose>0) {
//        //    printf("survFunction for y = [ ");
//        //    for(size_t nd=0; nd<DIM_MAX; nd++) {
//        //        printf("%f, ", ec->neuron->y[nd]);
//        //    }
//        //    printf("] = [ "); 
//        //    for(size_t nd=0; nd<NCOMP; nd++) {
//        //        printf(" %f, ", pp[nd]);
//        //    }                
//        //    printf(" ]\n");
//        //}
//        return 0;
//    }

    int EntropyCalc::Integrand(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata) {
        EntropyCalc *ec = (EntropyCalc*)userdata;
        double t_cur= -datum::inf;
        for(size_t nd = 0; nd<ec->n; nd++) {
            ec->neuron->y[nd] = ec->T0 + (ec->Tmax - ec->T0)*xx[nd];
            if(t_cur > ec->neuron->y[nd]) {
                ff[0] = 0;
                return 0;
            } 
            t_cur = ec->neuron->y[nd];
        }                       
        double p = survFunction(ec->neuron, ec->T0, ec->Tmax);
        ff[0] = p*log(p)*(ec->Tmax - ec->T0);
        
        if(ec->cuba_verbose>0) {
            printf("survFunction for y = [ ");
            for(size_t nd=0; nd< ec->n; nd++) {
                printf("%f, ", ec->neuron->y[nd]);
            }
            printf("] = %f \n", ff[0]); 
        }
        return 0;
    }
//    double EntropyCalc::entropy_fn_int(const double &fn, EntropyCalc *ec) {
//        if(ec->n_cur < ec->n-1) {
//            ec->neuron->y[ec->n_cur] = fn;
//            ec->n_cur += 1;
//            Log::Info << "We going to integrate entr [" << fn << "," << ec->Tmax << "]\n";
//            double H = int_trapezium<EntropyCalc>(fn, ec->Tmax, 100, ec, &entropy_fn_int);
//            double H = DEIntegrator<double, EntropyCalc>::Integrate(ec, &entropy_fn_int, fn, ec->Tmax, 1e-06);
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
//    double EntropyCalc::run() {
//       neuron->y.clean();
//       for(size_t ni_cur = 0; ni_cur<DIM_MAX; ni_cur++) {
//         neuron->y.push_back(T0);
//       }                
//       neuron->y.print();
//       int verbose, comp, nregions, neval, fail;
//       double integral[NCOMP], error[NCOMP], prob[NCOMP];
//       Vegas(DIM_MAX, NCOMP, Integrand, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.NStart, cs.NIncrease, cs.NBatch, cs.GridNo, NULL, &neval, &fail, integral, error, prob);       
//       if(cuba_verbose>0) {
//           printf("VEGAS RESULT:\tneval %d\tfail %d\n", neval, fail);
//           for(size_t di=0; di<NCOMP; di++) {
//               printf("VEGAS RESULT:\t%.8f +- %.8f\tp = %.3f\n", integral[di], error[di], prob[di]);
//           }
//       }            
//       double final = 0;
//       for(size_t di=0; di<NCOMP; di++) {
//            final += integral[di];
//       }
//       printf("final: %f\n", final);     
//       return final;
//    }
//    double EntropyCalc::run(size_t n_calc) {
//        neuron->y.clean();
//        for(size_t ni=0; ni<n_calc; ni++) {
//            neuron->y.push_back(T0);
//        }
//        int verbose, comp, nregions, neval, fail;
//        double integral, error, prob;
//        n = n_calc;
//        if(n_calc == 0) {
//            n_calc = 1;
//        }
//        Vegas(n_calc, 1, Integrand, this, 1e-3, 1e-12, cuba_verbose, 0, 0, 1000, 100, 1000, 100, 0, NULL, &neval, &fail, &integral, &error, &prob);
//        if(cuba_verbose>0) {
//            printf("VEGAS RESULT:\tneval %d\tfail %d\n", neval, fail);
//            printf("VEGAS RESULT:\t%.8f +- %.8f\tp = %.3f\n", integral, error, prob);
//        }            
//        return integral;
//    }
    void EntropyCalc::IntPerfomance() {
        double sum_err = 0;
        if(cs.method == "Vegas") {
            printf("VEGAS PARAM: EpsRel %e, EpsAbs %e, MinEval %d, MaxEval %d, NStart %d, NIncrease %d, NBatch %d, GridNo %d\n", cs.EpsRel, cs.EpsAbs,cs.MinEval, cs.MaxEval, cs.NStart, cs.NIncrease, cs.NBatch, cs.GridNo);
        } 
        if(cs.method == "Suave") {
            printf("SUAVE PARAM: EpsRel %e, EpsAbs %e, MinEval %d, MaxEval %d, NNew %d, Flatness %f\n", cs.EpsRel, cs.EpsAbs,cs.MinEval, cs.MaxEval, cs.NNew, cs.Flatness);
        }            
        if(cs.method == "Divonne") {
            printf("DIVONNE PARAM: EpsRel %e, EpsAbs %e, MinEval %d, MaxEval %d, Key1 %d, Key2 %d, Key3 %d, MaxPass %d, Border %f, MaxChiSq %f, MinDeviation %f, NGiven %d, ldx %s, NExtra %d\n", cs.EpsRel, cs.EpsAbs,cs.MinEval, cs.MaxEval, cs.Key1, cs.Key2, cs.Key3, cs.MaxPass, cs.Border, cs.    MaxChiSq, cs.MinDeviation, cs.NGiven, cs.LDXGiven.c_str(), cs.NExtra);
        }
        if(cs.method == "Cuhre") {
            printf("CUHRE PARAM: EpsRel %e, EpsAbs %e, MinEval %d, MaxEval %d, Key %d\n", cs.EpsRel, cs.EpsAbs,cs.MinEval, cs.MaxEval, cs.Key);
        }
        for(int n_calc=0; n_calc<5; n_calc++) { 
            neuron->y.clean();
            for(size_t ni=0; ni<n_calc; ni++) {
                neuron->y.push_back(T0);
            }
            int verbose, comp, nregions, neval, fail;
            double integral, error, prob;
            n = n_calc;
            int n_calc_cur = n_calc;
            if(n_calc_cur == 0) {
                n_calc_cur = 1;
            }
            if(cs.method == "Vegas") {
                Vegas(n_calc_cur, 1, Integrand, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.NStart, cs.NIncrease, cs.NBatch, cs.GridNo, NULL, &neval, &fail, &integral, &error, &prob);
                printf("VEGAS RESULT (ndim %d):\tneval %d\tfail %d\n", n_calc_cur, neval, fail);
                printf("VEGAS RESULT:\t%.8f +- %.8f\tp = %.3f\n", integral, error, prob);
            }
            if(cs.method == "Suave") {
                Suave(n_calc_cur, 1, Integrand, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.NNew, cs.Flatness, NULL, &nregions, &neval, &fail, &integral, &error, &prob);
                printf("SUAVE RESULT (ndim %d):\tnregions %d\tneval %d\tfail %d\n", n_calc_cur, nregions, neval, fail);
                printf("SUAVE RESULT:\t%.8f +- %.8f\tp = %.3f\n", integral, error, prob);

            }
            if(cs.method == "Divonne") {
                int ldx = 0;
                if(cs.LDXGiven == "NDIM") ldx = n_calc_cur;
                Divonne(n_calc_cur, 1, Integrand, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.Key1, cs.Key2, cs.Key3, cs.MaxPass, cs.Border, cs.MaxChiSq, cs.MinDeviation, cs.NGiven, ldx, NULL, cs.NExtra, NULL, NULL, &nregions, &neval, &fail, &integral, &error, &prob);
                printf("DIVONNE RESULT (ndim %d):\tnregions %d\tneval %d\tfail %d\n", n_calc_cur, nregions, neval, fail);
                printf("DIVONNE RESULT:\t%.8f +- %.8f\tp = %.3f\n", integral, error, prob);

            }           
            if(cs.method == "Cuhre") {
                Cuhre(n_calc_cur, 1, Integrand, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, cs.MinEval, cs.MaxEval, cs.Key, NULL, &nregions, &neval, &fail, &integral, &error, &prob);
                printf("Cuhre RESULT (ndim %d):\tnregions %d\tneval %d\tfail %d\n", n_calc_cur, nregions, neval, fail);
                printf("Cuhre RESULT:\t%.8f +- %.8f\tp = %.3f\n", integral, error, prob);

            }
            sum_err += error;
        }
        printf("final err: %f\n", sum_err);
  }

};    

