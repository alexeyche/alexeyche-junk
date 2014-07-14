

#include "constants.h"

#include <util/templates_clean.h>
#define T pLConst
#include <util/util_vector_tmpl.c>



Constants* createConstants(const char *filename) {
    Constants *c = (Constants*)malloc(sizeof(Constants));
    c->adex = (AdExConstants*) malloc( sizeof(AdExConstants) );
    c->res_stdp = (ResourceSTDPConstants*) malloc( sizeof(ResourceSTDPConstants) );
    c->preproc = (PreprocessConstants*) malloc( sizeof(PreprocessConstants) );
    c->pacemaker = (PacemakerConstants*) malloc( sizeof(PacemakerConstants) );
    c->lc = TEMPLATE(createVector,pLConst)();

    if (ini_parse(filename, file_handler, c) < 0) {
        printf("Can't load %s\n", filename);
        return(NULL);
    }
    c->__target_rate = c->target_rate/c->sim_dim;
    c->__pr = c->pr/c->sim_dim;
    if(c->determ) {
        c->beta = 1000;
//        c->r0 = 1000;
    }
    if((c->determ)&&(c->learn)) {
        printf("Can't learn anything in determenistic mode\n");
        exit(1);
    }
    return(c);
}
void deleteConstants(Constants *c) {
    TEMPLATE(deleteVector,ind)(c->layers_size);
    TEMPLATE(deleteVector,double)(c->net_edge_prob);
    TEMPLATE(deleteVector,double)(c->input_edge_prob);
    TEMPLATE(deleteVector,double)(c->output_edge_prob);
    TEMPLATE(deleteVector,double)(c->inhib_frac);
    TEMPLATE(deleteVector,double)(c->weight_per_neuron);
    TEMPLATE(deleteVector,double)(c->wmax);
    TEMPLATE(deleteVector,double)(c->weight_decay_factor);
    TEMPLATE(deleteVector,double)(c->lrate);
    free(c->adex);
    free(c->res_stdp);
    free(c->preproc);
    free(c);
}

indVector* indVectorParse(const char *vals) {
    indVector *v = TEMPLATE(createVector,ind)();
    char *token;
    char *string = strdup(vals);
    while ((token = strsep(&string, " ")) != NULL) {

        TEMPLATE(insertVector,ind)(v, atoi(token));
    }
    free(string);
    return(v);
}

doubleVector* doubleVectorParse(const char *vals) {
    doubleVector *v = TEMPLATE(createVector,double)();
    char *token;
    char *string = strdup(vals);
    while ((token = strsep(&string, " ")) != NULL) {
        TEMPLATE(insertVector,double)(v, atof(token));
    }
    free(string);
    return(v);
}             

void fillIndVector(indVector *v, const char *vals) {
    char *token;
    char *string = strdup(vals);
    while ((token = strsep(&string, " ")) != NULL) {

        TEMPLATE(insertVector,ind)(v, atoi(token));
    }
    free(string);
}

void fillDoubleVector(doubleVector *v, const char *vals) {
    char *token;
    char *string = strdup(vals);
    while ((token = strsep(&string, " ")) != NULL) {
        TEMPLATE(insertVector,double)(v, atof(token));
    }
    free(string);
}

#define FILL_LAYER_CONST(name,type) {       \
        type##Vector *v = type##VectorParse(value); \
        for(size_t i=0; i<v->size; i++) {   \
            checkLC(c,i);                   \
            getLC(c,i)->name = v->array[i]; \
        }                                   \
 
int file_handler(void* user, const char* section, const char* name,
                   const char* value)
{
    Constants* c = (Constants*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("preprocess", "mult")) {
        c->preproc->mult = atof(value);
    } else 
    if (MATCH("preprocess", "gain")) {
        c->preproc->gain = atof(value);
    } else 
    if (MATCH("preprocess", "sigma")) {
        c->preproc->sigma = atof(value);
    } else 
    if (MATCH("preprocess", "dt")) {
        c->preproc->dt = atof(value);
    } else 
    if (MATCH("srm neuron", "neuron_type")) {
        if(strcmp(value, "PoissonLayer") == 0) {
            c->neuron_type = EPoissonLayer;
        } else {
            printf("Can't neuron type %s\n", value);
            exit(1);
        }
    } else 
    if (MATCH("srm neuron", "e0")) {
        c->e0 = atof(value);
    } else 
    if (MATCH("srm neuron", "e_exc")) {
        c->e_exc = atof(value);
    } else 
     if (MATCH("srm neuron", "e_inh")) {
        c->e_inh = atof(value);
    } else 
    if (MATCH("srm neuron", "ts")) {
        c->ts = atof(value);
    } else 
    if (MATCH("srm neuron", "tm")) {
        c->tm = atof(value);
    } else 
    if (MATCH("srm neuron", "tsr")) {
        c->tsr = atof(value);
    } else 
    if (MATCH("srm neuron", "r0")) {
        c->r0 = atof(value);
    } else 
    if (MATCH("srm neuron", "alpha")) {
        c->alpha = atof(value);
    } else 
    if (MATCH("srm neuron", "beta")) {
        c->beta = atof(value);
    } else 
    if (MATCH("srm neuron", "u_tr")) {
        c->u_tr = atof(value);
    } else 
    if (MATCH("srm neuron", "gain_factor")) {
        c->gain_factor = atof(value);
    } else 
    if (MATCH("srm neuron", "pr")) {
        c->pr = atof(value);
    } else 
    if (MATCH("srm neuron", "u_rest")) {
        c->u_rest = atof(value);
    } else 
    if (MATCH("srm neuron", "ta")) {
        c->ta = atof(value);
    } else 
    if (MATCH("srm neuron", "tr")) {
        c->tr = atof(value);
    } else 
    if (MATCH("srm neuron", "tb")) {
        c->tb = atof(value);
    } else     
    if (MATCH("srm neuron", "qa")) {
        c->qa = atof(value);
    } else 
    if (MATCH("srm neuron", "qr")) {
        c->qr = atof(value);
    } else 
    if (MATCH("srm neuron", "qb")) {
        c->qb = atof(value);
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
    if (MATCH("sim", "learn")) {
        c->learn = strcmp(value, "true") == 0;
    } else 
    if (MATCH("sim", "determ")) {
        c->determ = strcmp(value, "true") == 0;
    } else 
    if (MATCH("sim", "target_neurons")) {
        c->target_neurons = strcmp(value, "true") == 0;
    } else 
    if (MATCH("sim", "M")) {
        c->M = atoi(value);
    } else 
    if (MATCH("net", "N")) {
        FILL_LAYER_CONST(N,ind)
    } else 
    if (MATCH("net", "net_edge_prob")) {
        fillDoubleVector(c->net_edge_prob, value);
    } else 
    if (MATCH("net", "input_edge_prob")) {
        fillDoubleVector(c->input_edge_prob, value);
    } else 
    if (MATCH("net", "output_edge_prob")) {
        fillDoubleVector(c->output_edge_prob, value);
    } else 
    if (MATCH("net", "weight_per_neuron")) {
        fillDoubleVector(c->weight_per_neuron, value);
    } else 
    if (MATCH("net", "inhib_frac")) {
        fillDoubleVector(c->inhib_frac, value);
    } else
    if (MATCH("net", "ws")) {
        c->ws = atof(value);
    } else 
    if (MATCH("net", "wmax")) {
        fillDoubleVector(c->wmax, value);
    } else 
    if (MATCH("net", "aw")) {
        c->aw = atof(value);
    } else 
    if (MATCH("net", "weight_var")) {
        c->weight_var = atof(value);
    } else 
    if (MATCH("net", "syn_delays_gain")) {
        c->syn_delays_gain = atof(value);
    } else 
    if (MATCH("net", "syn_delays_rate")) {
        c->syn_delays_rate = atof(value);
    } else 
    if (MATCH("net", "axonal_delays_gain")) {
        c->axonal_delays_gain = atof(value);
    } else 
    if (MATCH("net", "axonal_delays_rate")) {
        c->axonal_delays_rate = atof(value);
    } else 
    if (MATCH("optimal stdp", "tc")) {
        c->tc = atof(value);
    } else 
    if (MATCH("optimal stdp", "mean_p_dur")) {
        c->mean_p_dur = atof(value);
    } else 
    if (MATCH("optimal stdp", "target_rate")) {
        c->target_rate = atof(value);
    } else 
    if (MATCH("optimal stdp", "target_rate_factor")) {
        c->target_rate_factor = atof(value);
    } else 
    if (MATCH("resource stdp", "A+")) {
        c->res_stdp->Aplus = atof(value);
    } else 
    if (MATCH("resource stdp", "A-")) {
        c->res_stdp->Aminus = atof(value);
    } else 
    if (MATCH("resource stdp", "tau+")) {
        c->res_stdp->tau_plus = atof(value);
    } else 
    if (MATCH("resource stdp", "tau-")) {
        c->res_stdp->tau_minus = atof(value);
    } else 
    if (MATCH("resource stdp", "tau_res")) {
        c->res_stdp->tau_res = atof(value);
    } else 
    if (MATCH("optimal stdp", "weight_decay_factor")) {
        fillDoubleVector(c->weight_decay_factor, value);
    } else 
    if (MATCH("learn", "reinforcement")) {
        c->reinforcement = strcmp(value, "true") == 0;
    } else 
    if (MATCH("learn", "reward_baseline")) {
        c->reward_baseline = atof(value);
    } else 
    if (MATCH("learn", "trew")) {
        c->trew = atof(value);
    } else 
    if (MATCH("learn", "reward_ltp")) {
        c->reward_ltp = atof(value);
    } else 
    if (MATCH("learn", "tel")) {
        c->tel = atof(value);
    } else 
    if (MATCH("learn", "reward_ltd")) {
        c->reward_ltd = atof(value);
    } else 
    if (MATCH("learn", "lrate")) {
        fillDoubleVector(c->lrate, value);
    } else 
    if (MATCH("learn", "lrate")) {
        fillDoubleVector(c->lrate, value);
    } else 
    if (MATCH("learn", "lrate")) {
        fillDoubleVector(c->lrate, value);
    } else 
    if (MATCH("learn", "learning_rule")) {
        if(strcmp(value, "OptimalSTDP") == 0) {
            c->learning_rule = EOptimalSTDP;
        } else 
        if(strcmp(value, "ResourceSTDP") == 0) {
            c->learning_rule = EResourceSTDP;
        } else {
            printf("Can't find learning rule %s\n", value);
            exit(1);
        }
    } else 
    if (MATCH("learn", "p_set")) {
        c->p_set = atof(value);
    } else 
    if (MATCH("adex neuron", "C")) {
        c->adex->C = atof(value);
    } else 
    if (MATCH("adex neuron", "gL")) {
        c->adex->gL = atof(value);
    } else 
    if (MATCH("adex neuron", "EL")) {
        c->adex->EL = atof(value);
    } else 
    if (MATCH("adex neuron", "Vtr")) {
        c->adex->Vtr = atof(value);
    } else 
    if (MATCH("adex neuron", "slope")) { 
        c->adex->slope = atof(value);
    } else 
    if (MATCH("adex neuron", "tau_w")) {
        c->adex->tau_w = atof(value);
    } else 
    if (MATCH("adex neuron", "a")) {
        c->adex->a = atof(value);
    } else 
    if (MATCH("adex neuron", "b")) {
        c->adex->b = atof(value);
    } else
    if (MATCH("pacemaker", "amplitude")) {
        c->pacemaker->amplitude = atof(value);
    } else 
    if (MATCH("pacemaker", "cumulative_period_delta")) {
        c->pacemaker->cumulative_period_delta = atof(value);
    } else 
    if (MATCH("pacemaker", "frequency")) {
        c->pacemaker->frequency = atof(value);
    } else 
    if (MATCH("pacemaker", "pacemaker_on")) {
        c->pacemaker->pacemaker_on = strcmp(value, "true") == 0;
    } else {
        return(0);
    } 
    return(1);
}

LayerConstants* getLC(Constants *c, size_t i) {
    return(c->lc->array[i]);
}

void checkLC(Constants *c, size_t i) {
    if (c->lc->size <= i) {
        TEMPLATE(insertVector,pLConst)(c->lc, (LayerConstants*) malloc(sizeof(LayerConstants)));
    }
}
//
// for i in `sed -ne '11,44p' ./srm_sim/constants.h | cut -d ' ' -f6  | sort | uniq  | tr -d ';'`; do  echo "printf(\"$i: %f,\n\", c->$i);"; done
void printConstants(Constants *c) {
    printf("lrate: "); printDoubleVector(c->lrate);
    printf("alpha: %f,\n", c->alpha);
    printf("syn_delays_rate: %f,\n", c->syn_delays_rate);
    printf("syn_delays_gain: %f,\n", c->syn_delays_gain);
    printf("syn_delays_rate: %f,\n", c->syn_delays_rate);
    printf("axonal_delays_gain: %f,\n", c->axonal_delays_gain);
    printf("aw: %f,\n", c->aw);
    printf("beta: %f,\n", c->beta);
    printf("dt: %f,\n", c->dt);
    printf("duration: %f,\n", c->duration);
    printf("e0: %f,\n", c->e0);
    printf("e_exc: %f,\n", c->e_exc);
    printf("e_inh: %f,\n", c->e_inh);
    printf("epochs: %d,\n", c->epochs);
    printf("gain_factor: %f,\n", c->gain_factor);
    printf("inhib_frac: ");   printDoubleVector(c->inhib_frac);
    printf("M: %d,\n", c->M);
    printf("mean_p_dur: %f,\n", c->mean_p_dur);
    printf("N: "); printIndVector(c->layers_size);
    printf("net_edge_prob: ");   printDoubleVector(c->net_edge_prob);
    printf("input_edge_prob: ");   printDoubleVector(c->input_edge_prob);
    printf("output_edge_prob: ");   printDoubleVector(c->output_edge_prob);
    printf("pr: %f,\n", c->pr);
    printf("seed: %d,\n", c->seed);
    printf("sim_dim: %f,\n", c->sim_dim);
    printf("determ: %d,\n", c->determ);
    printf("learn: %d,\n", c->learn);
    printf("r0: %f,\n", c->r0);
    printf("tsr: %f,\n", c->tsr);
    printf("target_rate: %f,\n", c->target_rate);
    printf("target_rate_factor: %f,\n", c->target_rate_factor);
    printf("target_neurons: %d,\n", c->target_neurons);
    printf("tc: %f,\n", c->tc);
    printf("tm: %f,\n", c->tm);
    printf("ta: %f,\n", c->ta);
    printf("tr: %f,\n", c->tr);
    printf("tb: %f,\n", c->tb);
    printf("qa: %f,\n", c->qa);
    printf("qr: %f,\n", c->qr);
    printf("qb: %f,\n", c->qb);
    printf("u_tr: %f,\n", c->u_tr);
    printf("ts: %f,\n", c->ts);
    printf("u_rest: %f,\n", c->u_rest);
    printf("weight_decay_factor: ");   printDoubleVector(c->weight_decay_factor);
    printf("weight_var: %f,\n", c->weight_var);
    printf("weight_per_neuron: ");   printDoubleVector(c->weight_per_neuron);
    printf("ws: %f,\n", c->ws);
    printf("wmax: ");   printDoubleVector(c->wmax);
}
