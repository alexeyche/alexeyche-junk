#ifndef NEURONS_NEW_H
#define NEURONS_NEW_H

#include "neurons.h"

namespace srm {
//    class TreeElem {
//    #define LEVEL 5
//    public:
//        TreeElem() {
//           for(size_t ti=0; ti<LEVEL; ti++) {
//               idx.push_back(new T
//           }
//        }
//        std::vector<TreeElem> idx;
//        const double index_time;
//        TreeElem *right;
//        TreeElem *left;
//    };
//    class TreeIndex {
//    public:
//        const double INDEX_TIMES = { 5, 10 };
//        void push(const double &t, size_t ind) {
//           int idx0 = t/100; 
//           if(idx0 > te.size()-1) {
//            te.push_back(TreeElem()
//           }
//        }
//        std::vector<TreeElem> te;
//    };
//    class TTime2 : public std::vector<double> {
//    static constexpr double INDEX_TIME = 5;
//    public:
//        TTime2() : std::vector<double>(), inf_elem(-datum::inf), n_elem_real(0), idx_counter(0) { }
//        TTime2(const int &n) : std::vector<double>(n), inf_elem(-datum::inf), n_elem_real(n) { }
//        double& operator() (const size_t i) {
//            return std::vector<double>::operator[] (i);
//        }
//        void push_back(double v) {
//            int idx_i = v/INDEX_TIME;
//            std::vector<double>::push_back(v); 
//            ti.push(v, std::vector<double>::size()-1)
//            n_elem_real++;
//        }
//
//        int search(const double &t) {
//            int idx_i = t/INDEX_TIME; 
//        }
//        
//        double& first() {
//            if(n_elem_real>0) {
//                return std::vector<double>::operator[] (0);
//            } else {
//                return inf_elem;
//            }
//        }        
//        double& last(const double &t = datum::inf) {
//            if(n_elem_real>0) {
//                int ind = search(t);
//                if(ind>=0) {
//                    return std::vector<double>::operator[] (ind);
//                }
//            }
//            return inf_elem;
//        }
//        void print() {
//            vec time_vec(*this);
//            time_vec.print();
//        }
//        size_t n_elem(const double &t) {
//            if(n_elem_real>0) {
//                return(search(t)+1);
//            } else {
//                return 0;
//            }
//        }
//        void clean() {
//            std::vector<double>::clear();
//            n_elem_real = 0;
//        }
//    private:
//        size_t n_elem_real;
//        double inf_elem;
//        TreeIndex ti;
//    };    
//    
//    
//    class Neuron2 : public SimElement {
//        unsigned int genid() { static unsigned int ID = 0; return ID++; }
//    public:
//        typedef std::vector<Neuron2*> TInput;
//
//        Neuron2() : SimElement(false), id_num(genid()) { }
//        Neuron2(Neuron2 *n) : SimElement(false), id_num(genid()), w(n->w), in(n->in), y(n->y) {}
//        virtual ~Neuron2() {} 
//        unsigned int id() { return id_num; } 
//        
//        
//        void add_input(Neuron2 *n, double w_n) {
//           in.push_back(n);
//           w.push_back(w_n);
//        }
//        
//        TTime* get_y() {
//            return &y;
//        }
//
//        std::vector<double> w;
//        TInput in; 
//        TTime y;
//    private:                
//        unsigned int id_num;
//    };   
    class SrmNeuron2 : public StochasticNeuron {
    #define EPSP_WORK_WINDOW 40 
    #define NU_WORK_WINDOW 40
    public:
        static constexpr double ts = 3;  // ms
        static constexpr double tm = 10; //10; // ms
        
        static double epsp(double t, double fj);
        
        static constexpr double u_abs = -300; // mV
        static constexpr double u_r = -100;    // mV
        static constexpr double trf = 3.25;   // ms
        static constexpr double trs = 3;      // ms
        static constexpr double dr = 1;       // ms

        static double nu(double t, double fi);
       
        SrmNeuron2() { }
        SrmNeuron2(SrmNeuron2 *n) :  StochasticNeuron(n) { }
    
        static constexpr double u_rest = -70; //mV
        static constexpr double alpha = 0.65;
        static constexpr double beta = 0.65;
        static constexpr double tresh = -50; //mV

        double u(const double &t, TTime &y_given);

        double u(const double &t);

        double p(const double &t, TTime &y_given);

        double p(const double &t);       

    };
};

#endif
