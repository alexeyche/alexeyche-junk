
#include <math.h>
#include <util/util.h>
#include <util/util_vector.h>

#include "actor_sim.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

//#include "pthread_barrier.h"
#define P( condition ) {if( (condition) != 0 ) { printf( "\n FAILURE in %s, line %d\n", __FILE__, __LINE__ );exit( 1 );}}

size_t n_threads = 4; 

pthread_barrier_t barrier;

void* thread_routine(void* param) {
    int i = *(int*)param;
    printf("%d started\n", i);        
    for(double t=0; t<1000; t+=1) {
         printf("%d:%f\n", i, t);
         pthread_barrier_wait( &barrier );
    }
    return NULL;
}

int main(void){
    int i;
    pthread_attr_t attr;
    P( pthread_attr_init( &attr ) );
    P(pthread_barrier_init( &barrier, NULL, n_threads ) );

    int *thread_ids = (int *) malloc( n_threads * sizeof( int ) );
    for( int i = 0; i < n_threads; i++ ) 
        thread_ids[i] = i;

    pthread_t *threads = (pthread_t *) malloc( n_threads * sizeof( pthread_t ) );
    for( int i = 1; i < n_threads; i++ ) 
        P( pthread_create( &threads[i], &attr, thread_routine, &thread_ids[i] ) );
    
    thread_routine( &thread_ids[0] );
    for( int i = 1; i < n_threads; i++ ) 
        P( pthread_join( threads[i], NULL ) );

    P( pthread_barrier_destroy( &barrier ) );
    P( pthread_attr_destroy( &attr ) );
    free( thread_ids );
    free( threads );

    return 0;
}
