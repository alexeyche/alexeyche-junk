#include <core.h>

#include <args/prep_opts.h>

#include <util/spikes_list.h>
#include <util/io.h>
#include <util/util.h>

#include <constants.h>

int main(int argc, char **argv) {
    ArgOptionsPrep a = parsePrepOptions(argc, argv);
    Constants *c = createConstants(a.const_filename);

    pMatrixVector* ts_data = readMatrixListFromFile(a.input_file);
    pMatrixVector* ts_labels = readMatrixListFromFile(a.input_labels_file);
    assert(ts_labels->size == 1);
    assert(ts_labels->array[0]->nrow == ts_data->size);
    
    pMatrixVector *out_data = TEMPLATE(createVector,pMatrix)();
    assert(ts_data->size > 0);

    indVector *ts_indices = TEMPLATE(createVector,ind)();
    for(size_t ti=0; ti< ts_data->size; ti++) {
        TEMPLATE(insertVector,ind)(ts_indices, ti);    
    }
    srand(time(NULL));
    shuffleIndVector(ts_indices);
    printConstants(c);
    return(0.0);
} 
