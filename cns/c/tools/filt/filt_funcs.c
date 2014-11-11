

#include "filt_funcs.h"

double conv(size_t i, Matrix *y, Matrix *w) {
    double acc = 0.0;
    for(size_t ni=0; ni<w->nrow; ni++) {
//        printf("%zu: ", ni);
        size_t wi=0;
        int yi=i;
        for(; (wi<w->ncol) && (yi>=0); yi--, wi++) {
//            printf("y:%d:%3.3f w:%d:%3.3f, ", yi, wi,  getMatrixElement(y, ni, yi), getMatrixElement(w, ni, wi));
            acc += getMatrixElement(y, ni, yi) * getMatrixElement(w, ni, wi);
        }
//        printf("\n");
    }
    return acc;
}

void *error_routine(void *args) {
    FiltWorker *fw = (FiltWorker*)args;
    fw->acc = 0.0;
    for(size_t i=fw->first; i<fw->last; i++) {
        double t_w = conv(i, fw->y, fw->w); 
        fw->acc += (t_w - fw->target->array[i]) * (t_w - fw->target->array[i]);
    }
    return(NULL);
}

void *error_grad_routine(void *args) {
    FiltWorker *fw = (FiltWorker*)args;
    fw->dedw = createZeroMatrix(fw->w->nrow, fw->w->ncol);

    for(size_t i=fw->first; i<fw->last; i++) {
        double gr_val = -2*(fw->target->array[i] - conv(i, fw->y, fw->w));
        for(size_t wi=0; wi < fw->dedw->nrow; wi++) {
            int yi=i;
            size_t wj=0;
            for(; (wj < fw->dedw->ncol) && (yi>=0); wj++, yi--) {
                incMatrixElement(fw->dedw, wi, wj, gr_val * getMatrixElement(fw->y, wi, yi));
            }
        }
    }
    return(NULL);
}

double calcError(Matrix *y, Matrix *w, doubleVector *target, int jobs) {
    assert(y->ncol == target->size);

    pthread_t *threads = (pthread_t *) malloc( jobs * sizeof( pthread_t ) );
    FiltWorker *workers = (FiltWorker*) malloc( jobs * sizeof(FiltWorker) );
    
    for(size_t wi=0; wi < jobs; wi++) {
        int points_per_thread = (y->ncol + jobs - 1) / jobs;
        workers[wi].first = min( wi    * points_per_thread, y->ncol );
        workers[wi].last  = min( (wi+1) * points_per_thread, y->ncol );
        workers[wi].y = y;
        workers[wi].w = w;
        workers[wi].target = target;
    }
    for( int i = 0; i < jobs; i++ )  {
        P( pthread_create( &threads[i], NULL, error_routine,  &workers[i]) );
    }
    for( int i = 0; i < jobs; i++ )  {
        P( pthread_join( threads[i], NULL) );
    }

    double E = 0.0;
    for( size_t wi=0; wi< jobs; wi++) {
        E += workers[wi].acc;
    }
    E /= target->size;

    free(threads);
    free(workers);

    return E;
}

Matrix* calcErrorGrad(Matrix *y, Matrix *w, doubleVector *target, int jobs) {
    assert(y->ncol == target->size);

    pthread_t *threads = (pthread_t *) malloc( jobs * sizeof( pthread_t ) );
    FiltWorker *workers = (FiltWorker*) malloc( jobs * sizeof(FiltWorker) );
    
    for(size_t wi=0; wi < jobs; wi++) {
        int points_per_thread = (y->ncol + jobs - 1) / jobs;
        workers[wi].first = min( wi    * points_per_thread, y->ncol );
        workers[wi].last  = min( (wi+1) * points_per_thread, y->ncol );
        workers[wi].y = y;
        workers[wi].w = w;
        workers[wi].target = target;
    }
    for( int i = 0; i < jobs; i++ )  {
        P( pthread_create( &threads[i], NULL, error_grad_routine,  &workers[i]) );
    }
    for( int i = 0; i < jobs; i++ )  {
        P( pthread_join( threads[i], NULL) );
    }

    Matrix *dedw = createZeroMatrix(w->nrow, w->ncol);
    for( size_t wi=0; wi< jobs; wi++) {
        for(size_t i=0; i<dedw->nrow; i++) {
            for(size_t j=0; j<dedw->ncol; j++) {
                incMatrixElement(dedw, i, j, getMatrixElement(workers[wi].dedw, i, j));
            }
        }
    }
    for(size_t i=0; i<dedw->nrow; i++) {
        for(size_t j=0; j<dedw->ncol; j++) {
            double dedw_acc = getMatrixElement(dedw, i, j);
            setMatrixElement(dedw, i, j, dedw_acc/target->size);
        }
    }

    free(threads);
    free(workers);

    return dedw;
}
