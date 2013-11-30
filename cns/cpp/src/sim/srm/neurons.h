#ifndef SRM_NEURONS_H
#define SRM_NEURONS_H

#include <cmath>
#include <sim/data/load.h>
#include <sim/socket/sim_socket_core.h>


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
            if(s == 0) { throw SrmException("Binary search on empty TTime\n"); }
            if (std::vector<double>::operator[] (0) > t) { return -1;}
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
        
        static double epsp(double t, double fj, double fi) {
            if( ((t-fj)<0) ||  (fj<0) ) return 0.0;
            if(fi>t) fi = -datum::inf;
            return (exp( -std::max(fi-fj, 0.0)/ts )/(1-ts/tm)) * (exp(-std::min(t-fi,t-fj)/tm) - exp(-std::min(t-fi,t-fj)/ts));
        }



        static constexpr double u_abs = -100; // mV
        static constexpr double u_r = -50;    // mV
        static constexpr double trf = 0.25;   // ms
        static constexpr double trs = 3;      // ms
        static constexpr double dr = 1;       // ms

        static double nu(double t, double fi) {
            if((t-fi)<0) return 0;
            if((t-fi)<dr) return u_abs;
            return u_abs*exp(-(t-fi+dr)/trf)+u_r*exp(-(t-fi)/trs);
        }
        
        
        SrmNeuron() { }
        SrmNeuron(SrmNeuron *n) :  StochasticNeuron(n) { }
    
        static constexpr double u_rest = -70; //mV
        static constexpr double alpha = 0.75;
        static constexpr double beta = 0.25; //0.25;
        static constexpr double tresh = -50; //mV

        double u(const double &t, TTime &y_given) {
            double epsp_pot = 0;
                        
            double &y_last = y_given.last(t-0.001);
//            printf("============================================\n");  
//            printf("y_last: %f\n", y_last); 
//            printf("in.size(): %d\n", n->in.size());  
            for(size_t i=0; i<in.size(); i++) {
                for(int j=(in[i]->y.n_elem(t)-1); j>=0; j--) {
                    //printf("epsp_pot: %e\n", epsp_pot);
                    //printf(" w: %e t: %f in.y(j): %f y: %f\n", w[i], t, in[i]->y(j), y_last);
                    if( (t - in[i]->y(j)) > EPSP_WORK_WINDOW) {
                        //printf("epsp ignoring: %e\n", epsp(t, in[i]->y(j), y_last));
                        continue;
                    }                        
                    epsp_pot += w[i]*epsp(t, in[i]->y(j), y_last);
                }
            }
            double nu_pot = 0;
            for(int i = (y_given.n_elem(t-0.001)-1); i>=0; i--) {
                if( (t-y_given(i)) > NU_WORK_WINDOW ) {
                    //printf("nu ignoring(%f): %e\n", t, nu(t, y_given(i)));
                    break;
                }                    
                nu_pot += nu(t, y_given(i));
            }
            return u_rest + epsp_pot + nu_pot;           
        }

        double u(const double &t) {
            return u(t, y); 
        }

        double p(const double &t, TTime &y_given) {
            double uc = u(t, y_given);
            return (beta/alpha)*(log(1+exp(alpha*(tresh-uc))) - alpha*(tresh-uc));           
        }
        double p(const double &t) {
            return p(t, y);           
        }
        
    };


}
#endif
