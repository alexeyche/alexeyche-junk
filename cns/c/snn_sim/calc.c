
#include "calc.h"



void* calcRoutine(void *args) {
    CalcWorker *cw = (CalcWorker*) args;
    int neuron_per_thread = (cw->probs->nrow + cw->nthreads - 1) / cw->nthreads;
    int first = min(  cw->thread_id    * neuron_per_thread, cw->probs->nrow );
    int last  = min( (cw->thread_id+1) * neuron_per_thread, cw->probs->nrow );
    Matrix *probs = cw->probs;
    Matrix *fired = cw->fired;
    
    for(size_t ni=first; ni < last; ni++) {
        size_t Yi=0;
        doubleVector **p_class = (doubleVector**) malloc(sizeof(doubleVector*)*cw->uniqClasses->size); 
        for(size_t ci=0; ci<cw->uniqClasses->size; ci++) {
            p_class[ci] = TEMPLATE(createVector,double)();
        }
        for(size_t i=0; i<(fired->ncol-cw->dur); i+=cw->dur) {
            double acc_p[cw->uniqClasses->size];
            size_t acc_it[cw->uniqClasses->size];
            for(size_t ci=0; ci<cw->uniqClasses->size; ci++) {
                acc_p[ci] = 0.0;
                acc_it[ci] = 0;
            }
            size_t local_Yi = 0;
            for(size_t pi=0; pi < (probs->ncol-cw->dur); pi+=cw->dur) {
                double p = 0;
                for(size_t k=0; k < cw->dur; k++) {
                    //double p_no_fire = exp(-getMatrixElement(probs, ni, pi+k));
                    double p_no_fire = 1-getMatrixElement(probs, ni, pi+k);
                    if(getMatrixElement(fired, ni, i+k) == 1) {
                        if(p_no_fire == 1.0) p_no_fire = 0.99999;
                        p += log(1-p_no_fire);
                    } else {
                        p += log(p_no_fire);
                    }
                    if(isinf(p)) {
                        printf("Inf found! p_no_fire == %f, prob == %f, fired == %f ni == %zu pi+k == %zu + %zu\n", p_no_fire, getMatrixElement(probs, ni, pi+k), getMatrixElement(fired, ni, i+k),  ni, pi,k);
                        printf("%f\n", fabs(p_no_fire - 0.0));
                        exit(1);
                    }
                }
                size_t ci = cw->classesIndices->array[local_Yi];
                acc_p[ci] += exp(p);
                acc_it[ci] += 1;

                local_Yi += 1;
            }
            for(size_t ci=0; ci<cw->uniqClasses->size; ci++) {
                TEMPLATE(insertVector,double)(p_class[ci], acc_p[ci]/acc_it[ci]);
            }
            Yi += 1;
        }
        Matrix *outm = createMatrix(cw->uniqClasses->size, cw->classesIndices->size);
        for(size_t ci=0; ci<cw->uniqClasses->size; ci++) {
//            printf("%zu, p_class[%zu]->size == %zu\n", ci, ci, p_class[ci]->size);
            for(size_t el_i=0; el_i < p_class[ci]->size; el_i++) {
//                printf("%zu %zu %zu : %f \n", ni, ci, el_i, p_class[ci]->array[el_i]);
                setMatrixElement(outm, ci, el_i, p_class[ci]->array[el_i]);
            }
            TEMPLATE(deleteVector,double)(p_class[ci]);
        }
        cw->out[ni] = outm;
        free(p_class);
    }
    pthread_barrier_wait( &barrier );
    return(NULL);
}


void calcRun(Matrix *fired, Matrix *probs, Matrix **out, intVector *uniqClasses, indVector *classesIndices, double dur, size_t jobs) {
    pthread_t *threads = (pthread_t *) malloc( jobs * sizeof( pthread_t ) );
    CalcWorker *workers = (CalcWorker*) malloc( jobs * sizeof(CalcWorker) );
    for(size_t wi=0; wi < jobs; wi++) {
        workers[wi].thread_id = wi;
        workers[wi].probs = probs;
        workers[wi].fired = fired;
        workers[wi].out = out;
        workers[wi].nthreads = jobs;
        workers[wi].dur = dur;
        workers[wi].uniqClasses = uniqClasses;
        workers[wi].classesIndices = classesIndices;
    }


    pthread_attr_t attr;
    P( pthread_attr_init( &attr ) );
    P( pthread_barrier_init( &barrier, NULL, jobs ) );
    for( int i = 1; i < jobs; i++ )  {
        P( pthread_create( &threads[i], &attr, calcRoutine,  &workers[i]) );
    }
    calcRoutine(&workers[0]);

    free(workers);
    free(threads);
}

