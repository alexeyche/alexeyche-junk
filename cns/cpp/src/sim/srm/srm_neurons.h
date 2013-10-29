#ifndef SRM_NEURONS_H
#define SRM_NEURONS_H


namespace srm {
    class TTime : public std::vector<double> {
    static constexpr double TIME_OF_FORGET = 200;
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
        double& last(size_t shift=0) {
            if(n_elem_real>0) {
                return std::vector<double>::operator[] (n_elem_real-1-shift);
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
            size_t shift = 0;
            while(t<last(shift)&&( (n_elem_real-forgotten_elems) > shift)) {
                shift++;
            }
            Log::Info << "for t=" << t << " last - " << last(shift) << "\n";
            return n_elem_real - forgotten_elems - shift;
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
        static constexpr double u_rest = -70; //mV
        static constexpr double alpha = 1;
        static constexpr double beta = 1;
        static constexpr double tresh = -50; //mV

        SrmNeuron() {
        }

        double u(const double &t); 
        double p(const double &t);
    };

    double prob(SrmNeuron *n);
}
#endif
