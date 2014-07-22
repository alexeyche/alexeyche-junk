
#include "prepare_ts.h"



Matrix* processThroughReceptiveFields(Matrix *ts, const double *centers, const Constants *c) {
    Matrix *ts_out = createMatrix(c->M, ts->ncol);
//    printf("%d %d %d", c->M, ts->nrow, c->M % ts->nrow);
    assert(c->M % ts->nrow == 0);
    
    size_t neurons_per_ts = c->M / ts->nrow;
    size_t i, ri;
    for(i=0, ri=0; (i < c->M) && (ri < ts->nrow); i+=neurons_per_ts, ri++) {
        for(size_t ni=i; ni<(i+neurons_per_ts); ni++) {
            for(size_t vi=0; vi < ts->ncol; vi++) {
                double val = getMatrixElement(ts, ri, vi);                
                double val_field = c->preproc->gain * exp( - ((centers[ni] - val) * (centers[ni] - val))/c->preproc->sigma );
                setMatrixElement(ts_out, ni, vi, val_field);
            }
        }
    }
    return(ts_out);
}

pMatrixVector* processTimeSeriesSet(pMatrixVector *v, const Constants *c) {
    double area_max = DBL_MIN;
    double area_min = DBL_MAX;
    for(size_t i=0; i < v->size; i++) {    
        Matrix *ts = v->array[i];
        for(size_t ts_i=0; ts_i < ts->nrow; ts_i++) {
            for(size_t vi=0; vi < ts->ncol; vi++) {
                double val = getMatrixElement(ts, ts_i, vi);
                if(val > area_max) area_max = val;
                if(val < area_min) area_min = val;
            }
        }
    }
    double centers[c->M];
    for(size_t ni=0; ni<c->M; ni++) {
        centers[ni] = area_min + (ni+0.5)*(area_max-area_min)/c->M;
    }

    pMatrixVector *out = TEMPLATE(createVector,pMatrix)();
    for(size_t i=0; i<v->size; i++) {
        TEMPLATE(insertVector,pMatrix)(out, processThroughReceptiveFields(v->array[i], centers, c));
    }
    return(out);
}


