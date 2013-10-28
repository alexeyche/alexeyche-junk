#ifndef SRM_H
#define SRM_H


namespace srm {
    const double sec = 1000; //ms

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

    class TTime : public std::vector<double> {
    static const double TIME_OF_FORGET = 200;
    public:
        TTime() : std::vector<double>(), inf_elem(-datum::inf), n_elem_real(0), forgotten_elems(0) { }
        double& operator() (const size_t i) {
            return std::vector<double>::operator[] (i+forgotten_elems);
        }
        void push_back(double v) {
            std::vector<double>::push_back(v);
            n_elem_real++;
        }
        double& first() {
            if(n_elem_real>0) {
                return std::vector<double>::operator[] (forgotten_elems);
            } else {
                return inf_elem;
            }
        }        
        double& last() {
            if(n_elem_real>0) {
                return std::vector<double>::operator[] (n_elem_real-1);
            } else {
                return inf_elem;
            }
        }
        void print() {
            vec time_vec(*this);
            time_vec.print();
        }
        size_t n_elem(const double &t) {
            if(n_elem_real-forgotten_elems == 0) { return 0; }
            while((t-first()>TIME_OF_FORGET)&&( (n_elem_real-forgotten_elems) > 0)) {
                forgotten_elems++;
            }
            return n_elem_real - forgotten_elems;
        }

        
    private:
        size_t n_elem_real;
        double inf_elem;
        size_t forgotten_elems;
    };
    
    class Neuron {
        unsigned int genid() { static unsigned int ID = 0; return ID++; }
    public:
        typedef std::vector<Neuron*> TInput;

        Neuron() : id_num(genid()) { }
        virtual ~Neuron() {} 
        unsigned int id() { return id_num; } 
        
        void add_input(Neuron *n, double w_n) {
           in.push_back(n);
           w.set_size(w.n_elem+1);
           w(w.n_elem-1) = w_n;
        }
        TTime* get_y() {
            return &y;
        }
        vec w;
        TInput in; 
        TTime y;
    private:                
        unsigned int id_num;
    };
    
    class DetermenisticNeuron: public Neuron {
    public:
        DetermenisticNeuron(TTime yv) { Neuron::y = yv; }
        DetermenisticNeuron(std::string s) { 
            vec y_arm(s);    
            for(size_t i=0; i<y_arm.n_elem; i++) {
                Neuron::y.push_back(y_arm(i));
            }
        }    
    };

    class StochasticNeuron : public Neuron {
    public:
        virtual double p (const double &t) = 0;
    };

    class PoissonNeuron : public StochasticNeuron {
    public:
        PoissonNeuron(double rate) : rate(rate) {}

        double p(const double &t) {
            return rate;
        }

        double rate;        
    };

    class SrmNeuron : public StochasticNeuron {
    public:
        static const double u_rest = -70; //mV
        static const double alpha = 1;
        static const double beta = 1;
        static const double tresh = -50; //mV

        SrmNeuron() {
        }

        double u(const double &t) {
            double epsp_pot = 0;
            double nu_pot = 0;
            for(size_t i=0; i<in.size(); i++) {
                for(size_t j=0; j< in[i]->y.n_elem(t); j++) {
//                    Log::Info << "epsp_pot: " << epsp_pot; 
//                    Log::Info << " w:" << w[i] << " t:"  << t << " in.y(j):" << in[i]->y(j) << " y:"  << y.last() << "\n";
                    epsp_pot += w(i)*srm::epsp(t, in[i]->y(j), y.last());
                }
            }
            for(size_t i=0; i<y.n_elem(t); i++) {
                nu_pot += srm::nu(t, y(i));
            }
            return u_rest + epsp_pot + nu_pot;
        }
        
        double p(const double &t) {
            double uc = u(t);
            return (beta/alpha)*(log(1+exp(alpha*(tresh-uc))) - alpha*(tresh-uc));
        }
    };

};


#endif
