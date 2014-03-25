#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdlib.h>
#include <string.h>

#include <util/util_vector.h>
#include <util/ini.h>


typedef struct {
    double e0;
    double ts;
    double tm;
    double ta;
    
    double alpha;
    double beta;
    double tr;
    
    double gain_factor;
    double pr;

    double u_rest;
    double duration;
    double dt;
    double sim_dim;
    unsigned int seed;

    int M;
    indVector *layers_size; 
    double net_edge_prob;
    double inhib_frac;
    
    int net_neurons_for_input;
    int afferent_per_neuron;
    double tc;
    double mean_p_dur;
    
    double target_rate;
    double target_rate_factor;
    double weight_per_neuron;
    double weight_decay_factor;

    double added_lrate;
    int epochs;
} Constants;

static int file_handler(void* user, const char* section, const char* name, const char* value);
Constants* createConstants(const char *filename);
void printConstants(Constants *c); 
void deleteConstants(Constants *c);



#endif
