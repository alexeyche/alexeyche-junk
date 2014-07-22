#ifndef POSTPROC_H
#define POSTPROC_H



#include <util/spikes_list.h>
#include <util/matrix.h>
#include <math.h>
#include <float.h>

#include <pthread.h>

#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}
#ifndef max
    #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
    #define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

pthread_barrier_t barrier;


typedef struct ClassificationStat {
    Matrix *confM;
    double rate;
    double NMI;
} ClassificationStat;

typedef struct KernelMatrixCalcWorker {
    size_t thread_id;
    size_t nthreads;
    pMatrixVector *left;
    pMatrixVector *right;
    Matrix *K;
} KernelMatrixCalcWorker;

void* kernelMatrixCalcJob(void *args);


pMatrixVector *calcHists(SpikesList *spikes, doubleVector *timeline, double ts_dur, double ksize);
indVector* firstNNClassify(Matrix *K, indVector *train_labels_ind);
ClassificationStat getClassificationStat(pMatrixVector *train, indVector *train_labels_ind, pMatrixVector *test, indVector *test_labels_ind, intVector *uniq_classes, int jobs);
double euclDistance(Matrix *m1, Matrix *m2);

#endif
