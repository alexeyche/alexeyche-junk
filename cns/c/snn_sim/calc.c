
#include "calc.h"


void* calcRoutine(void *args) {
    CalcWorker *cw = (CalcWorker*) args;
    int neuron_per_thread = (cw->probs->nrow + cw->nthreads - 1) / cw->nthreads;
    int first = min(  cw->thread_id    * neuron_per_thread, cw->probs->nrow );
    int last  = min( (cw->thread_id+1) * neuron_per_thread, cw->probs->nrow );
    Matrix *probs = cw->probs;
    Matrix *fired = cw->fired;
    Matrix *out = cw->out;

    for(size_t ni=first; ni < last; ni++) {
        size_t Yi=0;
        for(size_t i=0; i<(fired->ncol-cw->dur); i+=cw->dur) {
            size_t el_i = 0;
            double acc_p = 0; 
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
                el_i += 1;
                acc_p += exp(p);
            }
//            printf("<P %zu> == %f\n", Yi, acc_p/el_i);
            setMatrixElement(out, ni, Yi, acc_p/el_i);
            Yi += 1;
        }
    }
    pthread_barrier_wait( &barrier );
    return(NULL);
}


void calcRun(Matrix *fired, Matrix *probs, Matrix *out, double dur, size_t jobs) {
    pthread_t *threads = (pthread_t *) malloc( jobs * sizeof( pthread_t ) );
    CalcWorker *workers = (CalcWorker*) malloc( jobs * sizeof(CalcWorker) );
    for(size_t wi=0; wi < jobs; wi++) {
        workers[wi].thread_id = wi;
        workers[wi].probs = probs;
        workers[wi].fired = fired;
        workers[wi].out = out;
        workers[wi].nthreads = jobs;
        workers[wi].dur = dur;
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

