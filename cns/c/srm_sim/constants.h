#ifndef CONSTANTS_H
#define CONSTANTS_H

typedef struct {
    double e0;
    double ts;
    double tm;
    double ta;
    
    double alpha;
    double beta;
    double tr;
    
    double pr;

    double u_rest;
    double duration;
    double dt;
    double sim_dim;

    int M;
    int N;
    double net_edge_prob;
    double inhib_frac;
    
    int net_neurons_for_input;
    int afferent_per_neuron;
    double weight_per_neuron;
    double tc;
    double mean_p_dur;
    
    double target_rate;
    double target_rate_factor;
    double weight_per_neuron;
    
    double added_lrate;
    int epochs;
} Constants;

#endif
