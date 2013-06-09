
#include "neurons.h"

#include <sim/util/rand/rand_funcs.h>

Neurons::Neurons(NeuronGroupOptions opts) : SimElem(opts.neurons_num, opts.neurons_num), n(opts.neurons_num), V_out(n)
{
    V_out.fill(0);
    axon = new AxonDelay(n, V_out);

    for(size_t i=0; i<opts.group_size(); i++) {        
        vec a_cur(opts[i]->num);
        vec b_cur(opts[i]->num);
        vec c_cur(opts[i]->num);
        vec d_cur(opts[i]->num);
        vec V_rest_cur(opts[i]->num);
        a_cur.fill(opts[i]->a);
        b_cur.fill(opts[i]->b);
        c_cur.fill(opts[i]->c);
        d_cur.fill(opts[i]->d);
        V_rest_cur.fill(opts[i]->V_rest);
        a = join_cols(a, a_cur);                    
        b = join_cols(b, b_cur);
        c = join_cols(c, c_cur);
        d = join_cols(d, d_cur);
        V_rest = join_cols(V_rest, V_rest_cur);
        axon->prepareMe(&opts[i]->axonOpts);    
        treshold = opts[i]->treshold;
        ind_distr[opts[i]->name] = linspace<uvec>(a.n_elem-opts[i]->num, a.n_elem-1, opts[i]->num);
    }
    V = V_rest;
    u = V_rest % b;
    Isyn.zeros(n);  
    fired.zeros();

    //permutate 
    // uvec indices = get_shuffled_indices(n);
    // a = a(indices);
    // b = b(indices);
    // c = c(indices);
    // d = d(indices);
    // V = V(indices);
    // u = u(indices);
}

void Neurons::computeMe(double dt) {        
    // reset, if fired.n_elem == 0, it just ignores
    
    V(fired) = c(fired);    
    u(fired) = u(fired)+d(fired);            
    V = V + dt * (0.04*square(V) + 5*V - V_rest*2 - u + Isyn);
    
    u = u + dt * a * (b*V - u);
    fired = find(V>treshold);
    V(fired).fill(treshold);
    V_out(fired).fill(treshold);
}

vec& Neurons::getInput() {
    return Isyn;
}

vec& Neurons::getOutput() {  
    return axon->getOutput();
}
