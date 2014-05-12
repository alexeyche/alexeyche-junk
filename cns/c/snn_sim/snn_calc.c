

#include <core.h>
#include <args/calc_opts.h>

#include <util/io.h>
#include <util/matrix.h>

int main(int argc, char **argv) {
	ArgOptionsCalc a = parseCalcOptions(argc, argv);
    
    pMatrixVector* stats = readMatrixList(a.stat_file);
    assert(stats->size >= 2);

//    int neuron_per_thread = (s->num_neurons + s->nthreads - 1) / s->nthreads;
//    int first = min(  sw->thread_id    * neuron_per_thread, s->num_neurons );
//    int last  = min( (sw->thread_id+1) * neuron_per_thread, s->num_neurons );
     
    Matrix *probs = stats->array[0];
    Matrix *fired = stats->array[1];
    
        
    TEMPLATE(deleteVector,pMatrix)(stats);
    return(0);
}    


