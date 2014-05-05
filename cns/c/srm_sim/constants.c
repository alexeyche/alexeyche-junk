

#include "constants.h"

Constants* createConstants(const char *filename) {
    Constants *c = (Constants*)malloc(sizeof(Constants));
    c->layers_size = TEMPLATE(createVector,ind)();
    c->net_edge_prob = TEMPLATE(createVector,double)();
    c->input_edge_prob= TEMPLATE(createVector,double)();
    c->output_edge_prob = TEMPLATE(createVector,double)();
    c->inhib_frac= TEMPLATE(createVector,double)();
    c->weight_per_neuron= TEMPLATE(createVector,double)();
    c->wmax = TEMPLATE(createVector,double)();
    c->weight_decay_factor = TEMPLATE(createVector,double)();
    c->added_lrate = TEMPLATE(createVector,double)();
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
    TEMPLATE(deleteVector,double)(c->added_lrate);
    free(c->input_spikes_filename);
    free(c);
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

int file_handler(void* user, const char* section, const char* name,
                   const char* value)
{
    Constants* c = (Constants*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("input", "input_spikes_filename")) {
        c->input_spikes_filename = (char*)malloc(strlen(value) +1);
        assert(c->input_spikes_filename);
        strcpy(c->input_spikes_filename, value);
    } else 
    if (MATCH("srm", "e0")) {
        c->e0 = atof(value);
    } else 
    if (MATCH("srm", "e_exc")) {
        c->e_exc = atof(value);
    } else 
     if (MATCH("srm", "e_inh")) {
        c->e_inh = atof(value);
    } else 
    if (MATCH("srm", "ts")) {
        c->ts = atof(value);
    } else 
    if (MATCH("srm", "tm")) {
        c->tm = atof(value);
    } else 
    if (MATCH("srm", "tsr")) {
        c->tsr = atof(value);
    } else 
    if (MATCH("srm", "r0")) {
        c->r0 = atof(value);
    } else 
    if (MATCH("srm", "alpha")) {
        c->alpha = atof(value);
    } else 
    if (MATCH("srm", "beta")) {
        c->beta = atof(value);
    } else 
    if (MATCH("srm", "u_tr")) {
        c->u_tr = atof(value);
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
    if (MATCH("srm", "ta")) {
        c->ta = atof(value);
    } else 
    if (MATCH("srm", "tr")) {
        c->tr = atof(value);
    } else 
    if (MATCH("srm", "tb")) {
        c->tb = atof(value);
    } else     
    if (MATCH("srm", "qa")) {
        c->qa = atof(value);
    } else 
    if (MATCH("srm", "qr")) {
        c->qr = atof(value);
    } else 
    if (MATCH("srm", "qb")) {
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
    if (MATCH("net", "M")) {
        c->M = atoi(value);
    } else 
    if (MATCH("net", "N")) {
        fillIndVector(c->layers_size, value);
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
        fillDoubleVector(c->weight_decay_factor, value);
    } else 
    if (MATCH("learn", "added_lrate")) {
        fillDoubleVector(c->added_lrate, value);
    } else 
    if (MATCH("learn", "learning_rule")) {
        if(strcmp(value, "OptimalSTDP") == 0) {
            c->learning_rule = EOptimalSTDP;
        } else {
            printf("Can't find learning rule %s\n", value);
            exit(1);
        }
    } else 
    if (MATCH("learn", "p_set")) {
        c->p_set = atof(value);
    } else {
        return(0);
    } 
    return(1);
}



// for i in `sed -ne '11,44p' ./srm_sim/constants.h | cut -d ' ' -f6  | sort | uniq  | tr -d ';'`; do  echo "printf(\"$i: %f,\n\", c->$i);"; done
void printConstants(Constants *c) {
    printf("added_lrate: "); printDoubleVector(c->added_lrate);
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
    printf("input_spikes_filename: %s,\n", c->input_spikes_filename);
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
