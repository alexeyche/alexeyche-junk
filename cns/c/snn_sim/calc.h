#ifndef CALC_H
#define CALC_H

struct {
    Matrix *probs;
    Matrix *fired;

    size_t thread_id;
    size_t ni_first;
    size_t ni_last;
} CalcWorker;
#endif
