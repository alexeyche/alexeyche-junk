
#ifndef NEURONS_H
#define NEURONS_H

#include <sim/core.h>
#include <map>

using namespace sim;

#include "basic_elements.h"
#include "axon.h"

struct NeuronOptions {
    NeuronOptions(int num, const std::string &name, double a = 0.02, double b = 0.2, double c = -65, double d = 6, double V_rest = -70, double treshold = 30) : 
                  num(num), name(name), a(a), b(b), c(c), d(d), treshold(treshold), V_rest(V_rest), axonOpts(num) { }
    int num;
    const std::string name;
    double a;
    double b;
    double c;
    double d;
    double treshold;
    double V_rest;         
    AxonDelayOptions axonOpts;
};

class NeuronGroupOptions {  
public:    
    NeuronGroupOptions() : neurons_num(0), cur_treshold(0) {}
    void add(NeuronOptions *no) { 
            if (neuron_opts.size() > 0) {
                if (cur_treshold != no->treshold) {
                    Log::Warn << "Can't add NeuronOption - threshold varibale need to be homohenous" << std::endl;
                }                                    
            }  

            neuron_opts.push_back(no); 
            
            neurons_num += no->num;
            if (neuron_opts.size() == 1) {
                cur_treshold = no->treshold;                
            }            
        }
    int group_size() { return neuron_opts.size(); }      
    
    NeuronOptions* operator [] (int i) { return neuron_opts[i]; }
    int neurons_num;
    
private:    
    std::vector<NeuronOptions*> neuron_opts;
    double cur_treshold;
    
    // Создавать аксон скорее всего здесь + проверять соответствие типов
};

class Neurons : public SimElem<vec, vec> {
    public:
        Neurons(NeuronGroupOptions opts);
        void computeMe(double dt);  
        
        vec& getInput();
        vec& getOutput();

        uvec getIndSubgroup(const std::string &name) {
            return ind_distr[name];
        }
        uvec getIndAll() {
            return linspace<uvec>(0, n-1, n);
        }
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
        vec V_out;
        
        AxonDelay* axon;

    private:
        uvec fired;
        std::map<const std::string, uvec> ind_distr; 
        //system inf
               
};

#endif
