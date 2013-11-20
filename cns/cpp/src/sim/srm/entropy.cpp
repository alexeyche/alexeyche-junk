#include <sim/int/simple_int.h>


#include "entropy.h"
#include "research.h"

#include <cuba.h>

namespace srm {
    int EntropyCalc::IntegrandFull(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata) {
        EntropyCalc *ec = (EntropyCalc*)userdata;
        double pp[*ncomp];
        double t_cur= -datum::inf;
        for(size_t nd = 0; nd< *ndim; nd++) {
            ec->neuron->y[nd] = ec->T0 + (ec->Tmax - ec->T0)*xx[nd];
            if(t_cur > ec->neuron->y[nd]) {
                for(size_t nni=0; nni< *ncomp; nni++) // wrong order
                    ff[nni] = 0;
                return 0;
            } 
            t_cur = ec->neuron->y[nd];
        }                       
        survFunctionSeq(ec->neuron, ec->T0, ec->Tmax, pp);
        for(size_t ci=0; ci< *ncomp; ci++) {
            ff[ci] = pp[ci]*log(pp[ci])*(ec->Tmax - ec->T0);           
        }            
        
        if(ec->cs.VerboseInt) {
            printf("survFunction for y = [ ");
            for(size_t nd=0; nd< *ndim; nd++) {
                printf("%f, ", ec->neuron->y[nd]);
            }
            printf("] = [ "); 
            for(size_t nd=0; nd< *ncomp; nd++) {
                printf(" %f, ", pp[nd]);
            }                
            printf(" ]\n");
        }
        return 0;
    }

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
        if(p == 0) { ff[0] = 0; return 0; } 
        ff[0] = p*log(p);
         
        if(ec->cs.VerboseInt) {
            printf("survFunction for y = [ ");
            for(size_t nd=0; nd< ec->neuron->y.size(); nd++) {
                printf("%f, ", ec->neuron->y[nd]);
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
            return p*log(p);
        }            
    }
    double EntropyCalc::run(int dim = DIM_MAX) {
        double int_full = 0;
        double p0 = survFunction(neuron, T0, Tmax);
        int_full += p0*log(p0);
        for(int n_calc= 1; n_calc<=dim; n_calc++) { 
            neuron->y.clean();
            for(size_t ni=0; ni<n_calc; ni++) {
                neuron->y.push_back(T0);
            }
            int verbose, comp, nregions, neval, fail;
            double integral, error, prob;
            n = n_calc;
            
            cs.MaxEval = 3000; 
            cs.EpsAbs = 1e-06;
            Vegas(n_calc, 1, Integrand, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.NStart, cs.NIncrease, cs.NBatch, cs.GridNo, NULL, &neval, &fail, &integral, &error, &prob);           
            int_full += integral;
        } 
        return int_full;
    }
    
    double EntropyCalc::IntPerfomance() {
       
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
        double integral_full[DIM_MAX+1], error_full[DIM_MAX+1], prob_full[DIM_MAX+1];
        int neval_full[DIM_MAX+1], fail_full[DIM_MAX+1];
        // in case with no spikes
        double p0 = survFunction(neuron, T0, Tmax);
        integral_full[0] = p0 * log(p0);
        error_full[0] = 0; prob_full[0] = 0; neval_full[0] = 0; fail_full[0] = 0;

        for(int n_calc= cs.FullInt ? (DIM_MAX-1) : 1 ; n_calc<DIM_MAX+1; n_calc++) { 
            neuron->y.clean();
            for(size_t ni=0; ni<n_calc; ni++) {
                neuron->y.push_back(T0);
            }
            int n_comp = 1;
            auto inter = Integrand;
            if(cs.FullInt) { inter = IntegrandFull; n_comp=DIM_MAX; }
            
            int verbose, comp, nregions, neval[n_comp], fail[n_comp];
            double integral[n_comp], error[n_comp], prob[n_comp];
            
            n = n_calc;
            int n_calc_cur = n_calc;
            if(n_calc_cur == 0) {
                n_calc_cur = 1;
            }

            if(cs.method == "Vegas") {
                Vegas(n_calc_cur, n_comp, inter, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.NStart, cs.NIncrease, cs.NBatch, cs.GridNo, NULL, neval, fail, integral, error, prob);
                Log::Info << "ncomp: " << n_comp << "Vegas !! " <<  integral[0] << "\n";
            }
            if(cs.method == "Suave") {
                Suave(n_calc_cur, n_comp, inter, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.NNew, cs.Flatness, NULL, &nregions, neval, fail, integral, error, prob);
            }
            if(cs.method == "Divonne") {
                int ldx = 0;
                if(cs.LDXGiven == "NDIM") ldx = n_calc_cur;
                Divonne(n_calc_cur, n_comp, inter, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, 0, cs.MinEval, cs.MaxEval, cs.Key1, cs.Key2, cs.Key3, cs.MaxPass, cs.Border, cs.MaxChiSq, cs.MinDeviation, cs.NGiven, ldx, NULL, cs.NExtra, NULL, NULL, &nregions, neval, fail, integral, error, prob);
            }           
            if(cs.method == "Cuhre") {
                Cuhre(n_calc_cur, n_comp, inter, this, cs.EpsRel, cs.EpsAbs, cuba_verbose, cs.MinEval, cs.MaxEval, cs.Key, NULL, &nregions, neval, fail, integral, error, prob);
            }
            if(cs.method == "brute") {
                n_cur = 0;
                n = n_calc;
//                integral[0] = int_trapezium<EntropyCalc>(T0, Tmax, cs.NumEval, this, &entropy_fn_int);
                integral[0] = int_brute<EntropyCalc>(T0, Tmax, cs.Dt, this, &entropy_fn_int);
                error[0] = prob[0] = neval[0] = fail[0] = 0;
            }   
            if(cs.FullInt) {
                for(size_t ci=0; ci<n_comp; ci++) {
                    integral_full[ci] = integral[ci];
                    error_full[ci] = error[ci];
                    prob_full[ci] = prob[ci];
                    neval_full[ci] = neval[ci];
                    fail_full[ci] = fail[ci];
                }    
                break;
            } else {
                integral_full[n_calc] = integral[0];
                error_full[n_calc] = error[0];
                prob_full[n_calc] = prob[0];
                neval_full[n_calc] = neval[0];
                fail_full[n_calc] = fail[0];
            }
        }
        printf("%s RESULT (ndim %d):\n", cs.method.c_str(), DIM_MAX);
        double sum_err = 0;
        double sum_int = 0;
        for(size_t di=0; di<DIM_MAX+1; di++) {
            sum_err += error_full[di];
            sum_int += integral_full[di];
            printf("%s RESULT:\t%.8f +- %.8f\tp = %.3f\tneval %d\tfail %d\n", cs.method.c_str(), integral_full[di], error_full[di], prob_full[di], neval_full[di], fail_full[di]);
        }
        printf("final int: %f\n", sum_int);
        printf("final err: %f\n", sum_err);
        return sum_int;
  }

};    

