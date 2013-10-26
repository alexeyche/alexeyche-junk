

namespace srm {
    const double ts = 3; // ms
    const double tm = 10; // ms
    
    double epsp(double t, double fj, double fi) {
        if( ((t-fj)<0) ||  (fj<0) ) return 0.0;
        return (exp( -std::max(fi-fj, 0.0)/ts )/(1-ts/tm)) * (exp(-std::min(t-fi,t-fj)/tm) - exp(-std::min(t-fi,t-fj)/ts));
    }



    const double u_abs = -100; // mV
    const double u_r = -50; // mV
    const double trf = 0.25; // ms
    const double trs = 3; // ms
    const double dr = 1; // ms

    double nu(double t, double fi) {
        if((t-fi)<0) return 0;
        if((t-fi)<dr) return u_abs;
        return u_abs*exp(-(t-fi+dr)/trf)+u_r*exp(-(t-fi)/trs);
    }

    const double u_rest = -70; //mV
    class SrmNeuron {

    public:
        typedef std::vector<SrmNeuron*> TInput;
        SrmNeuron() {
        }

        double u(double t) {
            double epsp_pot = 0;
            double nu_pot = 0;
            for(size_t i=0; i<in.size(); i++) {
                Log::Info << "i:" << i << " ";
                for(size_t j=0; j< in[i]->y.n_elem; j++) {
                    if(y.n_elem>0) {
                        epsp_pot += w(i)*srm::epsp(t, in[i]->y(j), y.max());
                    } else {
                        epsp_pot += w(i)*srm::epsp(t, in[i]->y(j), -datum::inf);
                    }
                    Log::Info << "epsp_pot: " << epsp_pot << "\n";
                }
            }
            for(size_t i=0; i<y.n_elem; i++) {
                nu_pot += srm::nu(t, y(i));
            }
            return u_rest + epsp_pot + nu_pot;
        }

        vec* get_y() {
            return &y;
        }
        void add_input(SrmNeuron *n, double w_n) {
           in.push_back(n);
           w << w_n;
        }
        vec w;
        vec y;
        TInput in;
    };

};



