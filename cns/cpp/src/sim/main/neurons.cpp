
#include "neurons.h"

#include <sim/util/rand_funcs.hpp>

Neurons::Neurons(NeuronGroupOptions opts) 
{
    for(size_t i=0; i<opts.size(); i++) {
        n += opts[i].num;
        vec a_cur(opts[i].num);
        vec b_cur(opts[i].num);
        vec c_cur(opts[i].num);
        vec d_cur(opts[i].num);
        vec V_rest_cur(opts[i].num);
        a_cur.fill(opts[i].a);
        b_cur.fill(opts[i].b);
        c_cur.fill(opts[i].c);
        d_cur.fill(opts[i].d);
        V_rest_cur.fill(opts[i].V_rest);
        a = join_cols(a, a_cur);                    
        b = join_cols(b, b_cur);
        c = join_cols(c, c_cur);
        d = join_cols(d, d_cur);
        V_rest = join_cols(V_rest, V_rest_cur);
    }
    V = V_rest;
    u = V_rest % b;
    Isyn.zeros();  
    need_reset.zeros();

    //permutate 
    uvec indices = get_shuffled_indices(n);
    a = a(indices);
    b = b(indices);
    c = c(indices);
    d = d(indices);
    V = V(indices);
    u = u(indices);
}

void Neurons::computeMe(double dt) {        
    // reset, if need_reset.n_elem == 0, it just ignores
    V(need_reset) = c(need_reset);
    u(need_reset) = u(need_reset)+d(need_reset);        

    V = V + dt * (0.04*square(V) + 5*V - V_rest*2 - u + Isyn);
    u = u + dt * a * (b*V - u);
    need_reset = find(V>treshold);
    V(need_reset).fill(treshold);
}

void Neurons::setInput(vec I) {    
    Isyn = I;
}

vec Neurons::getOutput() {    
    return V;
}
