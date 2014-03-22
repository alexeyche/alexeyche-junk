#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdlib.h>
#include <string.h>


#include "util/ini.h"

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
    int N;
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

static int file_handler(void* user, const char* section, const char* name,
                   const char* value)
{
    Constants* c = (Constants*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("srm", "e0")) {
        c->e0 = atof(value);
    } else 
    if (MATCH("srm", "ts")) {
        c->ts = atof(value);
    } else 
    if (MATCH("srm", "tm")) {
        c->tm = atof(value);
    } else 
    if (MATCH("srm", "ta")) {
        c->ta = atof(value);
    } else 
    if (MATCH("srm", "alpha")) {
        c->alpha = atof(value);
    } else 
    if (MATCH("srm", "beta")) {
        c->beta = atof(value);
    } else 
    if (MATCH("srm", "tr")) {
        c->tr = atof(value);
    } else 
    if (MATCH("srm", "gain_factor")) {
        c->gain_factor = atof(value);
    } else 
    if (MATCH("srm", "pr")) {
        c->pr = atof(value);
    } else 
    if (MATCH("srm", "u_rest")) {
        c->u_rest = atof(value);
    } else 
    if (MATCH("sim", "dt")) {
        c->dt = atof(value);
    } else 
    if (MATCH("sim", "sim_dim")) {
        c->sim_dim = atof(value);
    } else 
    if (MATCH("sim", "epochs")) {
        c->epochs = atof(value);
    } else 
    if (MATCH("sim", "duration")) {
        c->duration = atof(value);
    } else 
    if (MATCH("sim", "seed")) {
        c->seed = atoi(value);
    } else 
    if (MATCH("net", "M")) {
        c->M = atoi(value);
    } else 
    if (MATCH("net", "N")) {
        c->N = atoi(value);
    } else 
    if (MATCH("net", "net_edge_prob")) {
        c->net_edge_prob = atof(value);
    } else 
     if (MATCH("net", "net_neurons_for_input")) {
        c->net_neurons_for_input = atoi(value);
    } else 
    if (MATCH("net", "afferent_per_neuron")) {
        c->afferent_per_neuron = atoi(value);
    } else 
    if (MATCH("net", "weight_per_neuron")) {
        c->weight_per_neuron = atof(value);
    } else 
    if (MATCH("net", "inhib_frac")) {
        c->inhib_frac = atof(value);
    } else 
    if (MATCH("learn", "tc")) {
        c->tc = atof(value);
    } else 
    if (MATCH("learn", "mean_p_dur")) {
        c->mean_p_dur = atof(value);
    } else 
    if (MATCH("learn", "target_rate")) {
        c->target_rate = atof(value);
    } else 
     if (MATCH("learn", "target_rate_factor")) {
        c->target_rate_factor = atof(value);
    } else 
    if (MATCH("learn", "weight_decay_factor")) {
        c->weight_decay_factor = atof(value);
    } else 
    if (MATCH("learn", "added_lrate")) {
        c->added_lrate = atof(value);
    } else {
        return(0);
    } 
    return(1);
}

Constants* createConstants(const char *filename);
void printConstants(Constants *c); 
void deleteConstants(Constants *c);



#endif
