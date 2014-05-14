#ifndef CALC_H
#define CALC_H

#include <pthread.h>

#include <util/matrix.h>
#include <math.h>


#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}
#ifndef max
    #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
    #define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

pthread_barrier_t barrier;

typedef struct CalcWorker {
    Matrix *probs;
    Matrix *fired;
    Matrix *out;
    Matrix **out_full;
    intVector *uniqClasses;
    indVector *classesIndices;

    double dur;
    size_t thread_id;
    size_t nthreads;
    bool fullOut;
} CalcWorker;

void* calcRoutine(void *args);
void calcRun(Matrix *fired, Matrix *probs, Matrix *out, Matrix **out_full, intVector *uniqClasses, indVector *classesIndices, double dur, size_t jobs, bool fullOut);

#endif
