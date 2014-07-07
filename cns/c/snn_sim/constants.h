#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdlib.h>
#include <string.h>

#include <core.h>
#include <util/util_vector.h>
#include <util/ini.h>

typedef enum { EOptimalSTDP, EResourceSTDP } learning_rule_t;
typedef enum { EPoissonLayer } neuron_layer_t;

typedef struct {
    double C;
    double gL;
    double EL;
    double Vtr;
    double slope;
    double tau_w; 
    double a;
    double b;
} AdExConstants;

typedef struct {
    bool pacemaker_on;
    indVector *net_layer_ids;
    double frequency;
    double cumulative_period_delta;
    double amplitude;
} PacemakerConstants;

typedef struct {
    double Aplus;
    double Aminus;
    double tau_plus;
    double tau_minus;
    double tau_res;
} ResourceSTDPConstants;

typedef struct {
    double gain;
    double sigma;
    double dt;
    double mult;
} PreprocessConstants;

typedef struct {
    neuron_layer_t neuron_type;
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
    doubleVector *lrate;

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

    int epochs;
    double p_set;
    
    learning_rule_t learning_rule;
    AdExConstants *adex;
    ResourceSTDPConstants *res_stdp;
    PreprocessConstants *preproc;
    bool reinforcement;
    double reward_ltd;
    double reward_ltp;
    double reward_baseline;
    double tel;
    double trew;
    PacemakerConstants *pacemaker;
} Constants;

int file_handler(void* user, const char* section, const char* name,
                   const char* value);

Constants* createConstants(const char *filename);
void printConstants(Constants *c); 
void deleteConstants(Constants *c);
void fillIndVector(indVector *v, const char *vals);
void fillDoubleVector(doubleVector *v, const char *vals);

#endif
