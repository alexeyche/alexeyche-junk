#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdlib.h>
#include <string.h>

#include <core.h>
#include <util/util_vector.h>
#include <util/ini.h>

typedef enum { ENull, EOptimalSTDP, EResourceSTDP, ESimpleSTDP, ETripleSTDP } learning_rule_t;
typedef enum { EPoissonLayer, EWtaLayer, EAdaptLayer, EWtaAdaptLayer } neuron_layer_t;
typedef enum { EExpHennequin, ELinToyoizumi, EExpBohte, EExp } prob_fun_t;

#ifndef max
    #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

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
    double max_freq;
    double __max_freq;
} WtaConstants;

typedef struct {
    bool pacemaker_on;
    double frequency;
    double cumulative_period_delta;
    double amplitude;
} PacemakerConstants;

typedef struct {
    double tau_plus;
    double tau_minus;
    double tau_y;
    double Aplus;
    double __Aplus;
    double Aminus;
    double p_target;
    double __Aminus_cube_delim_p_target;
    double tau_average;
    double __sec_tau_average;
} TripleSTDPConstants;


typedef struct {
    double Aplus;
    double Aminus;
    double tau_plus;
    double tau_minus;
    double tau_res;
    double __Aplus_max_Amin;
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
    prob_fun_t prob_fun;
    bool learn;
    bool determ;
    double net_edge_prob0;
    double net_edge_prob1;
    size_t net_edge_prob_group_size;
    double input_edge_prob0;
    double input_edge_prob1;
    size_t input_edge_prob_group_size;
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
    double ltd_factor;
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
    bool target_neurons;
    
    int M;
    pLConstVector *lc;

    double target_rate;
    double __target_rate; // not to fill
    double target_rate_factor;

    int epochs;
    
    AdExConstants *adex;
    ResourceSTDPConstants *res_stdp;
    TripleSTDPConstants *tr_stdp;
    PreprocessConstants *preproc;
    bool reinforcement;
    double reward_ltd;
    double reward_ltp;
    double reward_baseline;
    double tel;
    double trew;
    PacemakerConstants *pacemaker;
    WtaConstants *wta;
} Constants;

// cat ./snn_sim/constants.c | grep -E "^[-a-zA-Z_*]+ " | sed -e 's/[ ]*{/;/g'
Constants* createConstants(const char *filename);
void deleteConstants(Constants *c);
indVector* indVectorParse(const char *vals);
doubleVector* doubleVectorParse(const char *vals);
pccharVector* pccharVectorParse(const char *vals);
neuron_layer_t neuronTypeParse(char *str);
learning_rule_t learningRuleParse(char *str);
bool boolParse(char *str);
int file_handler(void* user, const char* section, const char* name, const char* value);
LayerConstants* getLayerConstantsC(Constants *c, size_t i);
bool checkLC(Constants *c, size_t i, const char *field);
void doublePrint(double v);
void uintPrint(unsigned int v);
void intPrint(int v);
void boolPrint(bool v);
void AdExConstantsPrint(AdExConstants *c);
void ResourceSTDPConstantsPrint(ResourceSTDPConstants *c);
void PacemakerConstantsPrint(PacemakerConstants *c);
void PreprocessConstantsPrint(PreprocessConstants *c);
void pLConstVectorPrint(pLConstVector *v);
void size_tPrint(size_t v);
void learning_rule_tPrint(learning_rule_t v);
void neuron_layer_tPrint(neuron_layer_t v);
void LayerConstantsPrint(LayerConstants *c);
void printConstants(Constants *c);

#endif
