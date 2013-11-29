#include <sim/int/simple_int.h>


#include "entropy.h"
#include "research.h"


namespace srm {
//    int EntropyCalc::IntegrandFull(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata) {
//        EntropyCalc *ec = (EntropyCalc*)userdata;
//        double pp[*ncomp];
//        double t_cur= -datum::inf;
//        for(size_t nd = 0; nd< *ndim; nd++) {
//            ec->neuron->y[nd] = ec->T0 + (ec->Tmax - ec->T0)*xx[nd];
//            if(t_cur > ec->neuron->y[nd]) {
//                for(size_t nni=0; nni< *ncomp; nni++) // wrong order
//                    ff[nni] = 0;
//                return 0;
//            } 
//            t_cur = ec->neuron->y[nd];
//        }                       
//        survFunctionSeq(ec->neuron, ec->T0, ec->Tmax, pp);
//        for(size_t ci=0; ci< *ncomp; ci++) {
//            ff[ci] = -pp[ci]*log(pp[ci])*(ec->Tmax - ec->T0);           
//        }            
//        
//        if(ec->cs.VerboseInt) {
//            printf("survFunction for y = [ ");
//            for(size_t nd=0; nd< *ndim; nd++) {
//                printf("%f, ", ec->neuron->y[nd]);
//            }
//            printf("] = [ "); 
//            for(size_t nd=0; nd< *ncomp; nd++) {
//                printf(" %f, ", pp[nd]);
//            }                
//            printf(" ]\n");
//        }
//        return 0;
//    }

    int EntropyCalc::EntropyIntegrand(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata) {
        EntropyCalc *ec = (EntropyCalc*)userdata;
        
        TTime y(*ndim);
        double t_cur= -datum::inf;
        for(size_t nd = 0; nd< *ndim; nd++) {
            y[nd] = ec->T0 + (ec->Tmax - ec->T0)*xx[nd];
            if(t_cur > y[nd]) {
                ff[0] = 0;
                return 0;
            } 
            t_cur = y[nd];
        }                       
        double p = survFunction(ec->neuron, y, ec->T0, ec->Tmax);
        if(p == 0) { ff[0] = 0; return 0; } 
        ff[0] = -p*log(p);
         
        if(ec->cs.VerboseInt) {
            printf("survFunction for y = [ ");
            for(size_t nd=0; nd< y.size(); nd++) {
                printf("%f, ", y[nd]);
            }
            printf("] = %e  H = %e\n", p, ff[0]); 
        }
        ff[0] = ff[0]*(ec->Tmax - ec->T0);
        return 0;
    }
    double EntropyCalc::entropy_fn_int(const double &fn, EntropyCalc *ec) {
        if(ec->n_cur+1 < ec->n) {
            ec->neuron->y[ec->n_cur] = fn;
            ec->n_cur += 1;
//            Log::Info << "We going to integrate entr [" << fn << "," << ec->Tmax << "]\n";
            double H = int_brute<EntropyCalc>(fn, ec->Tmax, ec->cs.Dt, ec, &entropy_fn_int);
//            double H = int_trapezium<EntropyCalc>(fn, ec->Tmax, 1000, ec, &entropy_fn_int);
//            double H = int_trapezium<EntropyCalc>(ec->T0, ec->Tmax, 10, ec, &entropy_fn_int);
            //double H = DEIntegrator<double, EntropyCalc>::Integrate(ec, &entropy_fn_int, fn, ec->Tmax, 1e-06);
            ec->n_cur = 0;
//            Log::Info << "We integrated H = " << H << "\n"; 
            return H;
        } else {
            ec->neuron->y[ec->n_cur] = fn;
            double p = survFunction(ec->neuron, ec->T0, ec->Tmax);
            if(ec->cs.VerboseInt) {
                printf("survFunction for y = [ ");
                for(size_t ni=0; ni<ec->neuron->y.size(); ni++) {
                    printf(" %f, ", ec->neuron->y[ni]);
                }
                printf(" ] = %e H = %e\n", p, p*log(p));
            }
            if(p == 0) {
                return 0;
            }
            return -p*log(p);
        }            
    }
    void EntropyCalc::printIntConf() {
        if(cs.method == "Vegas") {
            printf("VEGAS PARAM: EpsRel %e, EpsAbs %e, MinEval %d, MaxEval %d, NStart %d, NIncrease %d, NBatch %d, GridNo %d\n", cs.EpsRel, cs.EpsAbs,cs.MinEval, cs.MaxEval, cs.NStart, cs.NIncrease, cs.NBatch, cs.GridNo);
        } 
        if(cs.method == "Suave") {
            printf("SUAVE PARAM: EpsRel %e, EpsAbs %e, MinEval %d, MaxEval %d, NNew %d, Flatness %f\n", cs.EpsRel, cs.EpsAbs,cs.MinEval, cs.MaxEval, cs.NNew, cs.Flatness);
        }            
        if(cs.method == "Divonne") {
            printf("DIVONNE PARAM: EpsRel %e, EpsAbs %e, MinEval %d, MaxEval %d, Key1 %d, Key2 %d, Key3 %d, MaxPass %d, Border %f, MaxChiSq %f, MinDeviation %f, NGiven %d, ldx %s, NExtra %d\n", cs.EpsRel, cs.EpsAbs,cs.MinEval, cs.MaxEval, cs.Key1, cs.Key2, cs.Key3, cs.MaxPass, cs.Border, cs.MaxChiSq, cs.MinDeviation, cs.NGiven, cs.LDXGiven.c_str(), cs.NExtra);
        }
        if(cs.method == "Cuhre") {
            printf("CUHRE PARAM: EpsRel %e, EpsAbs %e, MinEval %d, MaxEval %d, Key %d\n", cs.EpsRel, cs.EpsAbs,cs.MinEval, cs.MaxEval, cs.Key);
        }
    }    

    //const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata
    vec EntropyCalc::integrate(int ndim, int ncomp, int (integrand)(const int*, const double*, const int*, double*, void*) ) {
        int verbose, comp, nregions, neval, fail;
        double integral[ncomp], error[ncomp], prob[ncomp];       
        
        if(cs.CubaVerbose > 0) {
            printIntConf();
        }
        
        if(cs.method == "Vegas") {
            Vegas(ndim, ncomp, integrand, this, cs.EpsRel, cs.EpsAbs, cs.CubaVerbose, 0, cs.MinEval, cs.MaxEval, cs.NStart, cs.NIncrease, cs.NBatch, cs.GridNo, NULL, &neval, &fail, integral, error, prob);
        }
        if(cs.method == "Suave") {
            Suave(ndim, ncomp, integrand, this, cs.EpsRel, cs.EpsAbs, cs.CubaVerbose, 0, cs.MinEval, cs.MaxEval, cs.NNew, cs.Flatness, NULL, &nregions, &neval, &fail, integral, error, prob);
        }
        if(cs.method == "Divonne") {
            int ldx = 0;
            if(cs.LDXGiven == "NDIM") ldx = ndim;
            Divonne(ndim, ncomp, integrand, this, cs.EpsRel, cs.EpsAbs, cs.CubaVerbose, 0, cs.MinEval, cs.MaxEval, cs.Key1, cs.Key2, cs.Key3, cs.MaxPass, cs.Border, cs.MaxChiSq, cs.MinDeviation, cs.NGiven, ldx, NULL, cs.NExtra, NULL, NULL, &nregions, &neval, &fail, integral, error, prob);
        }           
        if(cs.method == "Cuhre") {
            Cuhre(ndim, ncomp, integrand, this, cs.EpsRel, cs.EpsAbs, cs.CubaVerbose, cs.MinEval, cs.MaxEval, cs.Key, NULL, &nregions, &neval, &fail, integral, error, prob);
        }
        return vec(integral, ncomp);
    }

    double EntropyCalc::run(int dim = DIM_MAX) {
        double int_full = 0;
        TTime y0; 
        double p0 = survFunction(neuron, y0, T0, Tmax);
        int_full += -p0*log(p0);
        for(int n_calc = 1; n_calc<=dim; n_calc++) { 
            vec integral = integrate(n_calc, 1, EntropyIntegrand);
            int_full += integral(0);
        } 
        return int_full;
    }
    
};    

