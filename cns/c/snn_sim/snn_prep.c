#include <core.h>
#include <time.h>

#include <args/prep_opts.h>

#include <util/spikes_list.h>
#include <util/io.h>
#include <util/util.h>

#include <constants.h>

int main(int argc, char **argv) {
    ArgOptionsPrep a = parsePrepOptions(argc, argv);
    Constants *c = createConstants(a.const_filename);

    pMatrixVector* ts_data = readMatrixListFromFile(a.input_file);
    assert(ts_data->size > 0);

    pMatrixVector* ts_labels_m = readMatrixListFromFile(a.input_labels_file);
    assert(ts_labels_m->size == 1);
    assert(ts_labels_m->array[0]->nrow == ts_data->size);

    doubleVector *ts_labels = doubleVectorFromMatrix(ts_labels_m->array[0]);

    SpikePatternsList *spl = runNeurons(ts_data, ts_labels, c, false);
    
    printConstants(c);
    
    return(0.0);
} 
