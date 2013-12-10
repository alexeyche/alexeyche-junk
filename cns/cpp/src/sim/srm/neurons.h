#ifndef SRM_NEURONS_H
#define SRM_NEURONS_H

#include <cmath>
#include <sim/data/load.h>


#include <functional>
#include "memoize.h"

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
        TTime(const int &n) : std::vector<double>(n), inf_elem(-datum::inf), n_elem_real(n), forgotten_elems(0) { }

        double& operator() (const size_t i) {
            return std::vector<double>::operator[] (i+forgotten_elems);
        }
        void push_back(double v) {
            std::vector<double>::push_back(v);
            n_elem_real++;
        }

        int binary_search(const double &t) {
            const size_t &s = std::vector<double>::size();
//            printf("DEBUG size : %zu\n", s);
            if(s == 0) { throw SrmException("Binary search on empty TTime\n"); }
            if (std::vector<double>::operator[] (0) > t) { return -1;}
            if (std::vector<double>::operator[] (s-1) <= t) { return s-1;}
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
                int ind = binary_search(t);
                if(ind>=0) {
                    return std::vector<double>::operator[] (ind);
                }
            }
            return inf_elem;
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
        void clean() {
            std::vector<double>::clear();
            n_elem_real = 0;
            forgotten_elems = 0;
        }
    private:
        size_t n_elem_real;
        double inf_elem;
        size_t forgotten_elems;
    };    

    class SimElement {
    public:
        SimElement(bool needPreCalc_v) : needPreCalc(needPreCalc_v) {}
        bool isNeedPreCalc() {
            return needPreCalc;
        }
        virtual void preCalculate(double T0, double Tmax, double dt) {}
    private:
        bool needPreCalc;
    };

    class Neuron : public SimElement {
        unsigned int genid() { static unsigned int ID = 0; return ID++; }
    public:
        typedef std::vector<Neuron*> TInput;

        Neuron() : SimElement(false), id_num(genid()) { }
        Neuron(Neuron *n) : SimElement(false), id_num(genid()), w(n->w), in(n->in), y(n->y) {}
        virtual ~Neuron() {} 
        unsigned int id() { return id_num; } 
        
        
        void add_input(Neuron *n, double w_n) {
           in.push_back(n);
           w.push_back(w_n);
        }
        
        TTime* get_y() {
            return &y;
        }

        std::vector<double> w;
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

    class StochasticNeuron : public Neuron {
    public:
        StochasticNeuron() {}
        StochasticNeuron(StochasticNeuron *n) : Neuron(n) {}
        virtual double u (const double &t) = 0;
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
    #define EPSP_WORK_WINDOW 40 
    #define NU_WORK_WINDOW 40
    public:
        static constexpr double ts = 3;  // ms
        static constexpr double tm = 10; //10; // ms
        
        static double epsp(double t, double fj, double fi);

        
        static constexpr double u_abs = -300; // mV
        static constexpr double u_r = -100;    // mV
        static constexpr double trf = 2.25;   // ms
        static constexpr double trs = 3;      // ms
        static constexpr double dr = 1;       // ms

        static double nu(double t, double fi);
       
        SrmNeuron(bool sl=false) : stdp_learning(sl) { }
        SrmNeuron(SrmNeuron *n) :  StochasticNeuron(n) { }
    
        static constexpr double u_rest = -70; //mV
        static constexpr double alpha = 1;//0.35;
        static constexpr double beta = 1;//0.15;
        static constexpr double tresh = -50; //mV

        double u(const double &t, TTime &y_given);

        double u(const double &t);

        double p(const double &t, TTime &y_given);

        double p(const double &t);       

        bool stdp_learning;
    };


}
#endif
