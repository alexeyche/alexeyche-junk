

#include "constants.h"

Constants* createConstants(const char *filename) {
    Constants *c = (Constants*)malloc(sizeof(Constants));
    c->layers_size = TEMPLATE(createVector,ind)();
    if (ini_parse(filename, file_handler, c) < 0) {
        printf("Can't load %s\n", filename);
        return(NULL);
    }
    c->__target_rate = c->target_rate/c->sim_dim;
    c->__pr = c->pr/c->sim_dim;
    if(c->determ) {
        c->beta = 1000;
    }
    if((c->determ)&&(c->learn)) {
        printf("Can't learn anything in determenistic mode\n");
        exit(1);
    }
    return(c);
}
void deleteConstants(Constants *c) {
    TEMPLATE(deleteVector,ind)(c->layers_size);
    free(c->input_spikes_filename);
    free(c);
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
    if (MATCH("net", "M")) {
        c->M = atoi(value);
    } else 
    if (MATCH("net", "N")) {
        TEMPLATE(insertVector,ind)(c->layers_size, atoi(value));
    } else 
    if (MATCH("net", "net_edge_prob")) {
        c->net_edge_prob = atof(value);
    } else 
    if (MATCH("net", "input_edge_prob")) {
        c->input_edge_prob = atof(value);
    } else 
    if (MATCH("net", "afferent_per_neuron")) {
        c->afferent_per_neuron = atoi(value);
    } else 
    if (MATCH("net", "ws")) {
        c->ws = atof(value);
    } else 
    if (MATCH("net", "wmax")) {
        c->wmax = atof(value);
    } else 
    if (MATCH("net", "aw")) {
        c->aw = atof(value);
    } else 
    if (MATCH("net", "weight_var")) {
        c->weight_var = atof(value);
    } else 
    if (MATCH("net", "weight_per_neuron")) {
        c->weight_per_neuron = atof(value);
    } else 
    if (MATCH("net", "inhib_frac")) {
        c->inhib_frac = atof(value);
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
        c->weight_decay_factor = atof(value);
    } else 
    if (MATCH("learn", "added_lrate")) {
        c->added_lrate = atof(value);
    } else {
        return(0);
    } 
    return(1);
}

// for i in `sed -ne '11,44p' ./srm_sim/constants.h | cut -d ' ' -f6  | sort | uniq  | tr -d ';'`; do  echo "printf(\"$i: %f,\n\", c->$i);"; done
void printConstants(Constants *c) {
    printf("added_lrate: %f,\n", c->added_lrate);
    printf("afferent_per_neuron: %d,\n", c->afferent_per_neuron);
    printf("alpha: %f,\n", c->alpha);
    printf("axonal_delays_rate: %f,\n", c->axonal_delays_rate);
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
    printf("inhib_frac: %f,\n", c->inhib_frac);
    printf("M: %d,\n", c->M);
    printf("mean_p_dur: %f,\n", c->mean_p_dur);
    printf("N: %d,\n", c->layers_size->array[0]);
    printf("net_edge_prob: %f,\n", c->net_edge_prob);
    printf("input_edge_prob: %f,\n", c->input_edge_prob);
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
    printf("weight_decay_factor: %f,\n", c->weight_decay_factor);
    printf("weight_var: %f,\n", c->weight_var);
    printf("weight_per_neuron: %f,\n", c->weight_per_neuron);
    printf("ws: %f,\n", c->ws);
    printf("wmax: %f,\n", c->wmax);
}
