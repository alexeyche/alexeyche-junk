#ifndef SRM_NEURONS_H
#define SRM_NEURONS_H

#include <cmath>
#include <sim/data/load.h>
#include <sim/socket/sim_socket_core.h>

namespace srm {
    struct SrmException : public std::exception
    {
       std::string s;
       SrmException(std::string ss) : s(ss) {}
       ~SrmException() throw () {} // Updated
       const char* what() const throw() { return s.c_str(); }
    };    
    
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

        size_t binary_search(const double &t) {
            const size_t &s = std::vector<double>::size();
            if(s == 0) { throw SrmException("Binary search on empty TTime\n"); }
            if (std::vector<double>::operator[] (0) > t) { return 0;}
            if (std::vector<double>::operator[] (s-1) < t) { return s-1;}
            size_t first = 0;
            size_t last= s;
            size_t mid = first + (last-first)/2;
            while(first < last) {
                if(t < std::vector<double>::operator[] (mid)) {
                    last = mid;
                } else {
                    first = mid+1;
                }
                mid = first + (last - first) / 2;
            }

            last--;
            return last;
        }
        double& first() {
            if(n_elem_real>0) {
                return std::vector<double>::operator[] (forgotten_elems);
            } else {
                return inf_elem;
            }
        }        
        double& last(const double &t = datum::inf) {
            if(n_elem_real>0) {
                return std::vector<double>::operator[] (binary_search(t));
            } else {
                return inf_elem;
            }
        }
        void print() {
            vec time_vec(*this);
            time_vec.print();
        }
        size_t n_elem(const double &t) {
            if(n_elem_real>0) {
                return(binary_search(t)+1);
            } else {
                return 0;
            }
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
        DetermenisticNeuron() {}
        DetermenisticNeuron(TTime yv) { Neuron::y = yv; }
        DetermenisticNeuron(std::string s) { 
            vec y_arm(s);    
            for(size_t i=0; i<y_arm.n_elem; i++) {
                Neuron::y.push_back(y_arm(i));
            }
        }
    };
    class TimeSeriesGroup {
    public:    
        struct TPattern {
            TPattern(size_t group_n, size_t n, double prob) : pattern(group_n, n), pattProb(prob) {}
            mat pattern;
            double pattProb;
        };
        TimeSeriesGroup(size_t n) { 
            for(size_t gi=0; gi<n; gi++) {
                group.push_back(DetermenisticNeuron());
            }
        }
        void loadPatternFromFile(std::string csv_file, double pattDur, double pattProb) {
            mat ts;
            data::Load(csv_file, ts, true, false); 
            for(size_t ci=0; ci<ts.n_cols; ci++) {
                TPattern p(group.size(), ts.n_rows, pattProb);
                double patt_dt = 0;
                double lb = min(ts.col(ci));
                double hb = max(ts.col(ci));
                double dt = (hb-lb)/(group.size()-1);
                for(size_t ri=0; ri<ts.col(ci).n_rows; ri++) {
                    double val = ts(ri, ci);                
                    int ind_n = (val-lb)/dt;
                    patt_dt += pattDur/ts.col(ci).n_rows;
                    p.pattern(ind_n, ri) = patt_dt;
                }
                patterns.push_back(p);
            }
        }
        
        std::vector<DetermenisticNeuron> group;
        std::vector<TPattern> patterns;
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
