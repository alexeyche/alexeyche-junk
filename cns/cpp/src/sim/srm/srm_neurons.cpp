
#include <sim/core.h>

#include "srm.h"
#include "srm_neurons.h"

namespace srm {

    double SrmNeuron::u(const double &t) {
        double epsp_pot = 0;
        double nu_pot = 0;
        for(size_t i=0; i<in.size(); i++) {
            for(size_t j=0; j< in[i]->y.n_elem(t); j++) {
    //                    Log::Info << "epsp_pot: " << epsp_pot; 
    //                    Log::Info << " w:" << w[i] << " t:"  << t << " in.y(j):" << in[i]->y(j) << " y:"  << y.last() << "\n";
                epsp_pot += w(i)*srm::epsp(t, in[i]->y(j), y.last(t));
            }
        }
        for(size_t i=0; i<y.n_elem(t); i++) {
            nu_pot += srm::nu(t, y(i));
        }
        return u_rest + epsp_pot + nu_pot;
    }
        
    double SrmNeuron::p(const double &t) {
        double uc = u(t);
        return (beta/alpha)*(log(1+exp(alpha*(tresh-uc))) - alpha*(tresh-uc));
    }

    double prob(const double &t, SrmNeuron *n) {
       return n->p(t);
    }  

};        
