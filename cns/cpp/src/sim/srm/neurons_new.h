#ifndef NEURONS_NEW_H
#define NEURONS_NEW_H

#include "neurons.h"

namespace srm {
    class TTime2 : public std::vector<double> {
    static constexpr double INDEX_TIME = 5;
    public:
        TTime2() : std::vector<double>(), inf_elem(-datum::inf), n_elem_real(0), idx_counter(0) { }
        TTime2(const int &n) : std::vector<double>(n), inf_elem(-datum::inf), n_elem_real(n) { }
        double& operator() (const size_t i) {
            return std::vector<double>::operator[] (i);
        }
        void push_back(double v) {
            int idx_i = v/INDEX_TIME;
            Log::Info << "idx_i = " << idx_i << " index.size() = " << index.size() << "\n"; 
            std::vector<double>::push_back(v);    // <- of that element
            
            if(idx_i == index.size()) {
                index.push_back(std::vector<double>::size()-1) ;
            }
            if(idx_i == index.size()-1) {
               index[idx_i] = std::vector<double>::size()-1;
            }
            if(idx_i < index.size()-1) {
                throw SrmException("Wrong insertion order in TTime\n");
            }
            n_elem_real++;
        }

        int search(const double &t) {
            int idx_i = t/INDEX_TIME; 
        }
        
        double& first() {
            if(n_elem_real>0) {
                return std::vector<double>::operator[] (0);
            } else {
                return inf_elem;
            }
        }        
        double& last(const double &t = datum::inf) {
            if(n_elem_real>0) {
                int ind = search(t);
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
                return(search(t)+1);
            } else {
                return 0;
            }
        }
        void clean() {
            std::vector<double>::clear();
            n_elem_real = 0;
        }
    private:
        size_t n_elem_real;
        double inf_elem;
        size_t idx_counter;
        std::vector<double> index;
    };    
    
    class Neuron2 : public SimElement {
        unsigned int genid() { static unsigned int ID = 0; return ID++; }
    public:
        typedef std::vector<Neuron2*> TInput;

        Neuron2() : SimElement(false), id_num(genid()) { }
        Neuron2(Neuron2 *n) : SimElement(false), id_num(genid()), w(n->w), in(n->in), y(n->y) {}
        virtual ~Neuron2() {} 
        unsigned int id() { return id_num; } 
        
        
        void add_input(Neuron2 *n, double w_n) {
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
    class SrmNeuron2 : Neuron2 {
    
    };
};

#endif
