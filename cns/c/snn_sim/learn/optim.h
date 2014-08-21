#ifndef LEARN_OPTIM_H
#define LEARN_OPTIM_H

#include <learn/learn.h>


typedef struct {
    learn_t base;
    
    indLList **learn_syn_ids;
    double **C;
    double *B;
    double *pacc;

    double time_passed;

    doubleVector **stat_B;
    doubleVector ***stat_C;
    double *eligibility_trace;

} OptimalSTDP;

typedef struct {
    learn_serial_t base;
    Matrix *pacc;
} OptimalSTDPSerial;

OptimalSTDP* init_OptimalSTDP(struct LayerPoisson *l);
void toStartValues_OptimalSTDP(learn_t *ls_t);
void propagateSynSpike_OptimalSTDP(learn_t *ls_t, const size_t *ni, const struct SynSpike *sp, const Constants *c);
void trainWeightsStep_OptimalSTDP(learn_t *ls_t, const double *u, const double *p, const double *real_p, const size_t *ni, const struct SimContext *s);
void resetValues_OptimalSTDP(learn_t *ls_t, const size_t *ni);
void free_OptimalSTDP(learn_t *ls_t);
void serialize_OptimalSTDP(learn_t *ls_t, FileStream *file, const struct Sim *s);
void deserialize_OptimalSTDP(learn_t *ls_t, FileStream *file, const struct Sim *s);
void saveStat_OptimalSTDP(learn_t *ls_t, FileStream *file);

#endif
