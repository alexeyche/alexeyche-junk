
#ifndef NEURONS_H
#define NEURONS_H

#include <sim/core.h>

using namespace sim;

#include "basic_elements.h"

struct NeuronOptions {
    NeuronOptions(int num, double a = 0.02, double b = 0.2, double c = -65, double d = 6, double V_rest = -70, double treshold = 30) : 
                  num(num), a(a), b(b), c(c), d(d), treshold(treshold), V_rest(V_rest) {}
    int num;
    double a;
    double b;
    double c;
    double d;
    double treshold;
    double V_rest;
};

class NeuronGroupOptions {  
public:    
    NeuronGroupOptions() : cur_treshold(0) {}
    void add(NeuronOptions no) { 
            if ((neuron_opts.size() > 0) && (cur_treshold != no.treshold)) { 
                Log::Warn << "Can't add NeuronOption - threshold varibale need to be homohenous" << std::endl;
            }  
            neuron_opts.push_back(no); 
            if (neuron_opts.size() == 1) {
                cur_treshold = no.treshold;
            }
        }
    int size() { return neuron_opts.size(); }      
    NeuronOptions operator [] (int i) { return neuron_opts[i]; }
    
private:    
    std::vector<NeuronOptions> neuron_opts;
    double cur_treshold;

};


class Neurons : public SimElem<vec, vec> {
    public:
        Neurons(NeuronGroupOptions opts);
        void computeMe(double dt);  
        
        void setInput(vec I);
        vec getOutput();

        int n;

        vec a;
        vec b;
        vec c;
        vec d;
        vec V_rest;
        double treshold;        
        vec u;
        vec Isyn;
        vec V;
        vec Vout;
    private:
        uvec need_reset;        
};

#endif
