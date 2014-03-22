

#include "constants.h"

Constants* createConstants(const char *filename) {
    Constants *c = (Constants*)malloc(sizeof(Constants));
    if (ini_parse(filename, file_handler, c) < 0) {
        printf("Can't load %s\n", filename);
        return(NULL);
    }
    return(c);
}
void deleteConstants(Constants *c) {
    free(c);
}
// for i in `sed -ne '11,44p' ./srm_sim/constants.h | cut -d ' ' -f6  | sort | uniq  | tr -d ';'`; do  echo "printf(\"$i: %f,\n\", c->$i);"; done
void printConstants(Constants *c) {
    printf("added_lrate: %f,\n", c->added_lrate);
    printf("afferent_per_neuron: %d,\n", c->afferent_per_neuron);
    printf("alpha: %f,\n", c->alpha);
    printf("beta: %f,\n", c->beta);
    printf("dt: %f,\n", c->dt);
    printf("duration: %f,\n", c->duration);
    printf("e0: %f,\n", c->e0);
    printf("epochs: %d,\n", c->epochs);
    printf("gain_factor: %f,\n", c->gain_factor);
    printf("inhib_frac: %f,\n", c->inhib_frac);
    printf("M: %d,\n", c->M);
    printf("mean_p_dur: %f,\n", c->mean_p_dur);
    printf("N: %d,\n", c->N);
    printf("net_edge_prob: %f,\n", c->net_edge_prob);
    printf("net_neurons_for_input: %d,\n", c->net_neurons_for_input);
    printf("pr: %f,\n", c->pr);
    printf("seed: %d,\n", c->seed);
    printf("sim_dim: %f,\n", c->sim_dim);
    printf("ta: %f,\n", c->ta);
    printf("target_rate: %f,\n", c->target_rate);
    printf("target_rate_factor: %f,\n", c->target_rate_factor);
    printf("tc: %f,\n", c->tc);
    printf("tm: %f,\n", c->tm);
    printf("tr: %f,\n", c->tr);
    printf("ts: %f,\n", c->ts);
    printf("u_rest: %f,\n", c->u_rest);
    printf("weight_decay_factor: %f,\n", c->weight_decay_factor);
    printf("weight_per_neuron: %f,\n", c->weight_per_neuron);
}
