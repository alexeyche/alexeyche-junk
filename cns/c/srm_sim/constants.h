#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdlib.h>
#include <string.h>

#include <core.h>
#include <util/util_vector.h>
#include <util/ini.h>


typedef struct {
    char *input_spikes_filename;
    
    double e0;
    double e_exc;
    double e_inh;
    double ts;
    double tm;
    double tsr;
    
    double alpha;
    double beta;
    double u_tr;
    double r0;
    
    double tr;
    double ta;
    double tb;
    
    double qr;
    double qa;
    double qb;
    
    double gain_factor;
    double pr;
    double __pr;
    double u_rest;

    double duration;
    double dt;
    double sim_dim;
    unsigned int seed;
    bool determ;
    bool learn;
    bool target_neurons;
    
    int M;
    indVector *layers_size; 
    doubleVector *net_edge_prob;
    doubleVector *input_edge_prob;
    doubleVector *output_edge_prob;
    doubleVector *inhib_frac;
    doubleVector *weight_per_neuron;
    doubleVector *wmax;
    doubleVector *weight_decay_factor;

    double tc;
    double mean_p_dur;
    double axonal_delays_rate;
    double axonal_delays_gain;
    double syn_delays_rate;
    double syn_delays_gain;


    double target_rate;
    double __target_rate; // not to fill
    double target_rate_factor;
    double ws;
    double aw;
    double weight_var;

    doubleVector *added_lrate;
    int epochs;
    double p_set;
} Constants;

int file_handler(void* user, const char* section, const char* name,
                   const char* value);

Constants* createConstants(const char *filename);
void printConstants(Constants *c); 
void deleteConstants(Constants *c);
void fillIndVector(indVector *v, const char *vals);
void fillDoubleVector(doubleVector *v, const char *vals);

#endif
