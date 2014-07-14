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
    size_t N;
    learning_rule_t learning_rule;
    neuron_layer_t neuron_type;
    bool learn;
    bool determ;
    double net_edge_prob;
    double input_edge_prob;
    double output_edge_prob;
    double inhib_frac;
    double weight_per_neuron;
    double wmax;
    double weight_decay_factor;
    double weight_var;
    double lrate;
    double axonal_delays_rate;
    double axonal_delays_gain;
    double syn_delays_rate;
    double syn_delays_gain;
    double ws;
    double aw;
} LayerConstants;

typedef LayerConstants lConst;
typedef lConst* pLConst;

#include <util/templates_clean.h>
#define T pLConst
#include <util/util_vector_tmpl.h>

typedef struct {
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
    double tc;
    double mean_p_dur;

    double duration;
    double dt;
    double sim_dim;
    unsigned int seed;
    bool determ;
    bool learn;
    bool target_neurons;
    
    int M;
    pLConstVector *lc;

    double target_rate;
    double __target_rate; // not to fill
    double target_rate_factor;

    int epochs;
    double p_set;
    
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

LayerConstants* getLC(Constants *c, size_t i);
void checkLC(Constants *c, size_t i);

#endif
