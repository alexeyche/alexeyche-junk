#ifndef ENTROPY_H
#define ENTROPY_H

namespace srm {

    class EntropyCalc {
    public:    
        EntropyCalc(SrmNeuron *neuron_v, double T0v, double Tmaxv) : neuron(neuron_v), T0(T0v), Tmax(Tmaxv), cuba_verbose(0) {
            const char *env = getenv("CUBAVERBOSE");
            if( env ) cuba_verbose = atoi(env);
        }
        double run(size_t n_calc);
        static int Integrand(const int *ndim, const double xx[], const int *ncomp, double ff[], void *userdata);
    private:        
        SrmNeuron *neuron;
        int n;
        double T0;
        double Tmax;
        int cuba_verbose;
    };

};    
#endif
